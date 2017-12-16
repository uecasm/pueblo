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

	This file consists of the Chaco DDE client interface to WEB clients.

----------------------------------------------------------------------------*/

//#include "grheader.h"
#include "headers.h"

#include <ChHTTP.h>
#include <ChDDE.h>
#include "ChDDEPrv.h"

//	Serialization, run time info, dyn create.
IMPLEMENT_SERIAL( CDDEObject, CObject, 1 )

#include <MemDebug.h>


//	Initialize static members.
CObList *CDDEObject::m_pcolRunning = 0;
DWORD CDDEObject::m_dwidInst;
HSZ CDDEObject::m_aTopics[CDDEObject::m_MaxTopics];

const char* CDDEObject::m_pstrServers[] =
			{
				"MOSAIC",
				"WEBSURF",
				"NETSCAPE",
				"IEXPLORE",
			};

char* CDDEObject::m_pstrWWWTopics[] =
			{
				"WWW_Activate",
				"WWW_Alert",
				"WWW_BeginProgress",
				"WWW_CancelProgress",
				"WWW_EndProgress",
				//"WWW_Exit",
				//"WWW_GetWindowInfo",
				//"WWW_ListWindows",
				"WWW_MakingProgress",
				"WWW_OpenURL",
				"WWW_ParseAnchor",
				"WWW_QueryURLFile",
				"WWW_QueryViewer",
				//"WWW_RegisterProtocol", 
				//"WWW_RegisterURLEcho",
				"WWW_RegisterViewer",
				//"WWW_RegisterWindowChange",
				"WWW_SetProgressRange",
				"WWW_ShowFile",
				//"WWW_UnRegisterProtocol",
				//"WWW_UnRegisterURLEcho",
				"WWW_UnRegisterViewer",
				//"WWW_UnRegisterWindowChange",
				//"WWW_URLEcho",
				"WWW_Version",
				"WWW_ViewDocFile",
				//"WWW_WindowChange",
				"WWW_QueryVersion",
				"WWW_OpenURLResult",
				"WWW_CancelTransaction",
				//"WWW_ViewDocCache",
				"WWW_RegisterNow",
				"WWW_RegisterDone",
			};

/////////////////////////////////////////////////////////////////////////////
// CDDEObject Construction

CDDEObject::CDDEObject() :  m_iServer( -1 ), m_dwRegTransactionID(0),
							m_hszBrowser(0)	
{
	m_pDDEConn = NULL;

	CommonConstruction( srvMosaic );
}

CDDEObject::CDDEObject( int iServer, ChHTTPDDE *pDDEConn) : m_iServer( -1 ),
			m_dwRegTransactionID(0), m_hszBrowser(0)
{
	ASSERT( iServer < maxServers );
	m_pDDEConn = pDDEConn;
	CommonConstruction( iServer );
}

/////////////////////////////////////////////////////////////////////////////
// CDDEObject Initialization

void CDDEObject::CommonConstruction( int iServer )	
{
	//	Init everything but the doc.
	//	That's been done in the actual constructor.
	
	
	//	Create the DDE service name that we'll be using in this object.
	m_csServiceName = DDE_SERVICE_NAME;
	
		
	//	If this is the only CDDEObject, then we need to load all topics
	m_hszServiceName = NULL;

	if ( m_pcolRunning == 0 )
	{
		m_pcolRunning = new CObList( 20 );
		ASSERT( m_pcolRunning );   

		//	Add us to the list of running objects.
 		m_rIndex = m_pcolRunning->AddTail(this);


		LoadTopics();
		
		//	Get the service name of the browser we're communicating with.
		SetServer( iServer );

	}
	else
	{
		//	Add us to the list of running objects.
		m_rIndex = m_pcolRunning->AddTail(this);
	}
	
	//	Register our service name.
	m_hszServiceName = DdeCreateStringHandle(m_dwidInst, 
				(char *)(const char *)m_csServiceName,	CP_WINANSI);
	DdeNameService(m_dwidInst, m_hszServiceName, NULL, DNS_REGISTER);
	GetDDEConn()->Display(ExplainError());
}

/////////////////////////////////////////////////////////////////////////////
// CDDEObject Destruction
//	
CDDEObject::~CDDEObject()	
{
	//	Disconnect every conversation that this object is handling.
	POSITION rTraverse = m_colConversations.GetHeadPosition();
	CDDEConversation *pConv;
	while(rTraverse != NULL)	
	{
		pConv = (CDDEConversation *)m_colConversations.GetNext(rTraverse);
		pConv->DoDisconnect();
	}

	//	Take ourselves out of the running object list.
	m_pcolRunning->RemoveAt(m_rIndex);
	
	//	Unregister our service name.
	DdeNameService(m_dwidInst, m_hszServiceName, NULL, DNS_UNREGISTER);
	DdeFreeStringHandle(m_dwidInst, m_hszServiceName);
	m_hszServiceName = NULL;
	
	//	If there are no more running objects, it's time to flush all the topics.
	if(m_pcolRunning->GetCount() == 0)	
	{	
		delete m_pcolRunning;
		m_pcolRunning = 0;

		FlushTopics();
		DdeFreeStringHandle(m_dwidInst, m_hszBrowser);
	}
}

void CDDEObject::SetServer( int iServer )
{
	if ( iServer == m_iServer )
	{	
		return;		
	}
	m_iServer = iServer;

	if ( m_hszBrowser )
	{
		DdeFreeStringHandle(m_dwidInst, m_hszBrowser);
	}

	m_hszBrowser = DdeCreateStringHandle(m_dwidInst, 
					(char *)m_pstrServers[iServer], CP_WINANSI);

}

/////////////////////////////////////////////////////////////////////////////
// CDDEObject Lookup
//	
//	
CDDEObject *CDDEObject::ResolveService(const char *pServiceName)	
{
	//	We need to go through each object that we have, and see if it is
	//		the service that we are looking for.
	POSITION rTraverse = m_pcolRunning->GetHeadPosition();
	CDDEObject *pTraverse = NULL;

	while(rTraverse != NULL)	
	{
		pTraverse = (CDDEObject *)m_pcolRunning->GetNext(rTraverse);
		if(pTraverse->m_csServiceName == pServiceName)	
		{
			break;
		}
		else	
		{
			pTraverse = NULL;
		}
	}
	
	return(pTraverse);
}

CDDEObject *CDDEObject::ResolveService(HSZ hszServiceName)	
{
	//	Convert the HSZ to a string to perform the lookup.
	DWORD dwLength = DdeQueryString(m_dwidInst, hszServiceName, NULL, 0,
		CP_WINANSI) + 1;
	char *pTemp = new char[dwLength];
	DdeQueryString(m_dwidInst, hszServiceName, pTemp, dwLength, CP_WINANSI);
	
	CDDEObject *pRetVal = ResolveService(pTemp);
	delete pTemp;
	return(pRetVal);
}

CDDEObject *CDDEObject::ResolveConversation(CDDEConversation *pConv)	
{
	//	We need to go through each object that we have, and see if it owns
	//		the conversation that we are looking for.
	POSITION rTraverse = m_pcolRunning->GetHeadPosition();
	CDDEObject *pTraverse = NULL;
	POSITION rTravConv = NULL;
	CDDEConversation *pTravConv = NULL;

	while(rTraverse != NULL)	
	{
		pTraverse = (CDDEObject *)m_pcolRunning->GetNext(rTraverse);
		
		rTravConv = pTraverse->m_colConversations.GetHeadPosition();
		while(rTravConv != NULL)	
		{
			pTravConv =
				(CDDEConversation *)pTraverse->m_colConversations.GetNext(rTravConv);
			if(pTravConv == pConv)	
			{
				return(pTraverse);
			}
		}		
	}
	
	return(NULL);
}

/////////////////////////////////////////////////////////////////////////////
// CDDEObject Connection management.
//	
HDDEDATA CDDEObject::ServerConnect(HSZ hszTopic)	
{
	//	just accept anything....
	return(DDETRUE);
}

/////////////////////////////////////////////////////////////////////////////
// CDDEObject Connection confirmation.
//	
void CDDEObject::ServerConnectConfirm(HCONV hConv)	
{
	//	Create a new conversation object, and add it to the conversations
	//		that this oject is handling.
	CDDEConversation *pConversation = new CDDEConversation(hConv);
	m_colConversations.AddTail(pConversation);
}

/////////////////////////////////////////////////////////////////////////////
// CDDEObject 
//	Disconnection of a conversation managment.
void CDDEObject::DDEDisconnect(CDDEConversation *pConv)	
{
	//	Go through every running DDE object, go through every conversation,
	//		until we find the correct conversation, and then remove it from
	//		the conversation list.
	//	Actual disconnection is taking place elsewhere....
	POSITION rTravObj = m_pcolRunning->GetHeadPosition();
	CDDEObject *pObject;
	POSITION rTravConv, rRemoveMe;
	while(rTravObj != NULL)	
	{
		pObject = (CDDEObject *)m_pcolRunning->GetNext(rTravObj);
		rTravConv = pObject->m_colConversations.GetHeadPosition();
		while(rTravConv != NULL)	
		{
			rRemoveMe = rTravConv;
			if(pConv == (CDDEConversation *)pObject->m_colConversations.GetNext(rTravConv))	
			{
				pObject->m_colConversations.RemoveAt(rRemoveMe);
				return;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDDEObject 
//	WWW topics
void CDDEObject::LoadTopics()	
{

	for ( int i = 0; i < m_MaxTopics; i++ )
	{
		m_aTopics[i] = DdeCreateStringHandle(m_dwidInst, m_pstrWWWTopics[i], CP_WINANSI);
	}	
}

void CDDEObject::FlushTopics()	
{
	for(int iTraverse = 0; iTraverse < m_MaxTopics; iTraverse++)	
	{
		DdeFreeStringHandle(m_dwidInst, m_aTopics[iTraverse]);
	}
}

int CDDEObject::EnumTopic(HSZ hszTopic)	
{
	//	Simply go through our held topics, and return the item which matches.
	//	Since HSZs are held by the system, handles should always match up.
	for(int iCounter = 0; iCounter < m_MaxTopics; iCounter++)	
	{
		if(hszTopic == m_aTopics[iCounter])	
		{
			break;
		}
	}
	
	if(iCounter != m_MaxTopics)	
	{
		return(iCounter);
	}
	return(-1);
}

//	Useful client/server methods.
CDDEConversation *CDDEObject::ClientConnect(int iTopic)	
{
	ASSERT(iTopic < m_MaxTopics);	
	HSZ hszTopic = m_aTopics[iTopic];
	
	//TRACE("Attempting conversation %d\n", iTopic);
	
	HCONV hConv = DdeConnect(m_dwidInst, m_hszBrowser, hszTopic, NULL);
	GetDDEConn()->Display(ExplainError());
	
	CDDEConversation *pRetVal = NULL;
	if(hConv != NULL)	
	{
		pRetVal = new CDDEConversation(hConv);
		m_colConversations.AddTail(pRetVal);
	}
	
	return(pRetVal);
}

HSZ CDDEObject::ClientArguments(const char *pFormat, ...)	
{
	//	Always pass in pointer values.
	//	This way, a NULL, can be interpreted as an optional, missing parameter.
	char *pTraverse = (char *)pFormat;	

	//	Variable number of arguments.
	va_list VarList;
	va_start(VarList, pFormat);

	char caNumpad[64];
	CString csBuffer;
	CString csRetval;
	
	while(*pTraverse)	{
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
			    //  See if we need to use hex.
			    #if( FALSE ) 
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
			
			if(pCS != NULL)	
			{
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
			
			if(pBool != NULL)	
			{
				if(*pBool != FALSE)	
				{
					csRetval += "TRUE";
				}
				else	
				{
					csRetval += "FALSE";
				}
			}
		}
		else	
		{
			//	Unhandled format, just get out of loop.
			ASSERT(0);
			break;
		}
		
		//	Go on to next type
		pTraverse = SkipToNextArgument(pTraverse);
		
		//	See if we need a comma
		if(*pTraverse != '\0')	
		{
			csRetval += ',';
		}
	}

	va_end(VarList);
	
	//	Make sure we're atleast returning something.
	if(csRetval.IsEmpty())	
	{
		return(NULL);
	}

	//	Return our resultant HSZ, created from our string.
	HSZ Final = DdeCreateStringHandle(m_dwidInst, (char *)
		(const char *)csRetval, CP_WINANSI);

	return(Final);
}

char *SkipToNextArgument(char *pFormat)	
{
	//	Safety dance
	if(pFormat == NULL || *pFormat == '\0')	
	{
		return(pFormat);
	}

	//	The next format is directly after a ','
	while(*pFormat != ',' && *pFormat != '\0')	
	{
		pFormat++;
	}
	if(*pFormat == ',')	
	{
		pFormat++;
	}
	
	return(pFormat);
}

char *ExtractArgument(HSZ hszArgs, int iArg)	
{
	//	Quoted strings are counted as only one argument, though
	//		the quotes are not copied into the return string.

	DWORD dwLength = DdeQueryString(CDDEObject::m_dwidInst, hszArgs, NULL, 0L,
		CP_WINANSI) + 1;
	char *pTraverse = new char[dwLength];
	char *pRemove = pTraverse;
	if(pTraverse == NULL)	
	{
		return(NULL);
	}	
	DdeQueryString(CDDEObject::m_dwidInst, hszArgs, pTraverse, dwLength, CP_WINANSI);
	
	//	safety dance
	if(*pTraverse == '\0' || iArg < 1)	
	{
		delete(pRemove);
		return(NULL);
	}
	
	//	Need to decrement the argument we're looking for, as the very
	//		first argument has no ',' at the beginning.
	iArg--;
	
	//	Search through the arguments, seperated by ','.
	while(iArg)	
	{
		//	Special handling of quoted strings.
		if(*pTraverse == '\"')	
		{
			//	Find the ending quote.
			while(*pTraverse != '\0')	
			{
				pTraverse++;
				if(*pTraverse == '\"')	
				{
					pTraverse++;	//	One beyond, please
					break;
				}
				else if(*pTraverse == '\\')	
				{
					//	Attempting to embed a quoted, perhaps....
					if(*(pTraverse + 1) == '\"')	
					{
						pTraverse++;
					}
				}
			}
		}
		while(*pTraverse != '\0' && *pTraverse != ',')	
		{
			pTraverse++;
		}

		//	Go beyond a comma
		if(*pTraverse == ',')	
		{
			pTraverse++;
		}
		
		iArg--;
		
		if(*pTraverse == '\0')
		{
			break;
		}
	}
	
	//	Handle empty arguments here.
	if(*pTraverse == ',' || *pTraverse == '\0')	
	{
		delete(pRemove);
		return(NULL);
	}
	
	int iLength = 1;
	char *pCounter = pTraverse;
	TwoByteBool bQuoted = FALSE;
	
	//	specially handle quoted strings
	if(*pCounter == '\"')	
	{
		pCounter++;
		bQuoted = TRUE;

		while(*pCounter != '\0')	
		{
			if(*pCounter == '\"')	
			{
				break;
			}
			else if(*pCounter == '\\')	
			{
				if(*(pCounter + 1) == '\"')	
				{
					pCounter++;
					iLength++;
				}
			}
			
			pCounter++;
			iLength++;
		}
	}
	while(*pCounter != '\0' && *pCounter != ',')	
	{
		iLength++;
		pCounter++;
	}
	
	//	Subtrace one to ignore ending quote if we were quoted....
	if(bQuoted == TRUE)	
	{
		iLength--;
	}
	
	//	Argument's of length 1 are of no interest.
	if(iLength == 1)	
	{
		delete(pRemove);
		return(NULL);
	}
	
	char *pRetVal = new char[iLength];
	
	if(*pTraverse == '\"')	
	{
		pTraverse++;
	}	
	strncpy(pRetVal, pTraverse, iLength - 1);
	pRetVal[iLength - 1] = '\0';
	
	delete(pRemove);
	return(pRetVal);
}

void CDDEObject::ServerReturned(HDDEDATA hArgs, const char *pFormat, ...)	
{
	//	Of course, only pointers should be passed in as the variable number of
	//		arguments so assignment can take place.
	//	hArgs is free'd off by this function.
	char *pDataArgs = (char *)DdeAccessData(hArgs, NULL);
	
	//	Initialize variable number of argumetns.
	va_list VarList;
	va_start(VarList, pFormat);	
	
	//	It will be possible that with only one argument,
	//		that we are receiving raw data intead of string notation.
	if(strchr(pFormat, ',') == NULL)	
	{
		//	Only one argument.
		if(strcmp(pFormat, "DW") == 0)	
		{
			DWORD *pWord;
			pWord = va_arg(VarList, DWORD *);
			*pWord = *(DWORD *)pDataArgs;
			DdeUnaccessData(hArgs);
			GetDDEConn()->Display(ExplainError());
			DdeFreeDataHandle(hArgs);
			GetDDEConn()->Display(ExplainError());
			va_end(VarList);
			return;
		}
		else if(strcmp(pFormat, "BL") == 0)	
		{
			TwoByteBool *pBool;
			pBool = va_arg(VarList, TwoByteBool *);
			*pBool = *(TwoByteBool *)pDataArgs;
			DdeUnaccessData(hArgs);
			GetDDEConn()->Display(ExplainError());
			DdeFreeDataHandle(hArgs);
			GetDDEConn()->Display(ExplainError());
			va_end(VarList);
			return;
		}
	}

	//	We are assuming NULL terminated data, since there is more than one
	//		parameter expected.
	HSZ hszArgs = DdeCreateStringHandle(m_dwidInst, pDataArgs,
		CP_WINANSI);
	DdeUnaccessData(hArgs);	
	DdeFreeDataHandle(hArgs);

	int i_ArgNum = 0;
	char *pScan = (char *)pFormat;
	char *pExtract;
	
	//	Loop through the arguments we are going to parse.
	while(pScan && *pScan)	
	{
		i_ArgNum++;
		pExtract = ExtractArgument(hszArgs, i_ArgNum);
	
		if(0 == strncmp(pScan, "DW", 2))	
		{
			//	DWORD.
			DWORD *pWord;
			pWord = va_arg(VarList, DWORD *);
			
			//	If there is nothing to scan, use a default value.
			if(pExtract == NULL)	
			{
				*pWord = 0x0;
			}
			else	
			{
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
				//	Fun thing about a qouted string, is that we need
				//		to compress and '\"' into '"'.
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
			else	
			{
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
			else	{
				//	Compare for a TRUE or a FALSE
				if(0 == stricmp(pExtract, "TRUE"))	
				{
					*pBool = TRUE;
				}
				else	
				{
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
	
	//	Free off our created HSZ string.
	DdeFreeStringHandle(m_dwidInst, hszArgs);
}

CString ExplainPoke(HDDEDATA hStatus)	
{
	switch((UINT)hStatus)	
	{
		case DDE_FACK:
			return(CString("ACK "));
		case DDE_FBUSY:
			return(CString("BUSY "));
		case DDE_FNOTPROCESSED:
			return(CString("NOTPROCESSED "));
	}
	
	return(CString("UNKNOWN "));
}

CString ExplainBool(TwoByteBool bFlag)	
{
	if(bFlag == FALSE)	
	{
		return(CString("FALSE "));
	}
	
	return(CString("TRUE "));
}

CString ExplainError()	
{
	UINT uError = DdeGetLastError(CDDEObject::m_dwidInst);
	
	switch(uError)	
	{
		case DMLERR_NO_ERROR:
			return(CString(""));
		case DMLERR_ADVACKTIMEOUT:
			return(CString("ADVACKTIMEOUT"));
		case DMLERR_BUSY:
			return(CString("BUSY"));
		case DMLERR_DATAACKTIMEOUT:
			return(CString("DATAACKTIMEOUT"));		
		case DMLERR_DLL_NOT_INITIALIZED:
			return(CString("DLL_NOT_INITIALIZED"));
		case DMLERR_DLL_USAGE:
			return(CString("DLL_USAGE"));		
		case DMLERR_EXECACKTIMEOUT:
			return(CString("EXECACKTIMEOUT"));
		case DMLERR_INVALIDPARAMETER:
			return(CString("INVALIDPARAMETER"));
		case DMLERR_LOW_MEMORY:
			return(CString("LOW_MEMORY"));
		case DMLERR_MEMORY_ERROR:
			return(CString("DMLERR_MEMORY_ERROR"));
		case DMLERR_NOTPROCESSED:
			return(CString("DMLERR_NOTPROCESSED"));
		case DMLERR_NO_CONV_ESTABLISHED:
			return(CString("NO_CONV_ESTABLISHED"));
		case DMLERR_POKEACKTIMEOUT:
			return(CString("POKEACKTIMEOUT"));
		case DMLERR_POSTMSG_FAILED:
			return(CString("POSTMSG_FAILED"));
		case DMLERR_REENTRANCY:
			return(CString("REENTRANCY"));
		case DMLERR_SERVER_DIED:
			return(CString("SERVER_DIED"));
		case DMLERR_SYS_ERROR:
			return(CString("SYS_ERROR"));
		case DMLERR_UNADVACKTIMEOUT:
			return(CString("UNADVACKTIMEOUT"));
		case DMLERR_UNFOUND_QUEUE_ID:
			return(CString("UNFOUND_QUEUE_ID"));
	}
	
	return(CString("Uknown Error"));
}

//	Client browser interface.
DWORD CDDEObject::WWW_Activate(DWORD dwWindowID, DWORD dwFlags)	
{
	DWORD dwActivatedID = 0ul;
	
	//	Make the connection.
	CDDEConversation *pConv = ClientConnect(m_Activate);
	if(pConv != NULL)	
	{	
		//	Construct the arguments.
		HSZ hszItem = ClientArguments("DW,DW", &dwWindowID, &dwFlags);
		
		//	Do the transaction.
		HDDEDATA hActivatedID = pConv->ClientRequest(hszItem);
		
		//	Convert the return value.
		if(hActivatedID != NULL)	
		{
			ServerReturned(hActivatedID, "DW", &dwActivatedID);
		}
	
		//	Cut the cord.
		pConv->DoDisconnect();
	}
	
	
	return(dwActivatedID);
}

void CDDEObject::WWW_RegisterDone( )	
{
	if ( srvMosaic == m_iServer )
	{
		CDDEConversation *pConv= ClientConnect(m_RegisterDone );
		if(pConv != NULL)	
		{	
			DWORD dwResult = 1;
			//	Construct the arguments.
			HSZ hszItem = ClientArguments("DW,DW", &m_dwRegTransactionID, &dwResult);
		
			//	Do the transaction.
			HDDEDATA hStatus = DDE_FNOTPROCESSED;
			hStatus = pConv->ClientPoke(hszItem);
		
			//	Cut the cord.
			pConv->DoDisconnect();
		}
	}

}


void CDDEObject::WWW_CancelProgress(DWORD dwTransactionID)	
{
	//	Make the connection.
	CDDEConversation *pConv;
	if ( srvMosaic == m_iServer )
	{
		pConv = ClientConnect(m_CancelTransaction );
	}
	else
	{
		pConv = ClientConnect(m_CancelProgress);
	}
	HDDEDATA hStatus = DDE_FNOTPROCESSED;
	
	if(pConv != NULL)	
	{	
		//	Construct the arguments.
		HSZ hszItem = ClientArguments("DW", &dwTransactionID);
		
		//	Do the transaction.
		hStatus = pConv->ClientPoke(hszItem);
		
		//	Cut the cord.
		pConv->DoDisconnect();
	}
	
	char aBuf[512];
	sprintf(aBuf, "WWW_CancelProgress(%lu);", dwTransactionID);
	GetDDEConn()->Display(aBuf);
}

#if 0
void CDDEObject::WWW_Exit()	
{
	//	Connect.
	CDDEConversation *pConv = ClientConnect(m_Exit);
	HDDEDATA hStatus = DDE_FNOTPROCESSED;
	
	if(pConv != NULL)	
	{
		//	Do the transaction.
		CString csTemp( m_pstrWWWTopics[m_Exit] );
		HSZ hszItem = ClientArguments("CS", &csTemp);
		hStatus = pConv->ClientPoke(hszItem);
		
		//	Cut the cord.
		pConv->DoDisconnect();
	}
	
	GetDDEConn()->Display(m_pstrWWWTopics[m_Exit]);

}
void CDDEObject::WWW_ListWindows()
{
	CDDEConversation *pConv = ClientConnect(m_ListWindows);
	HDDEDATA hWindows = NULL;
	
	if(pConv != NULL)	
	{
		CString csTemp( m_pstrWWWTopics[m_ListWindows] );
		HSZ hszItem = ClientArguments("CS", &csTemp);
		hWindows = pConv->ClientRequest(hszItem);
		
		pConv->DoDisconnect();
	}
	
	//	Nonstandard data (from the rest of the spec) we are parsing here.
	CString csWindows;
	
	if(hWindows != NULL)	
	{
		DWORD *pWindows = (DWORD *)DdeAccessData(hWindows, NULL);
		GetDDEConn()->Display(ExplainError());
		
		//	Go till NULL
		char aBuf[512];
		while(*pWindows)	
		{
			if(csWindows.IsEmpty() == FALSE)	
			{
				csWindows += ',';
			}
			sprintf(aBuf, "%lu", *pWindows);
			csWindows += aBuf;
			pWindows++;
		}
		DdeUnaccessData(hWindows);
		GetDDEConn()->Display(ExplainError());
		DdeFreeDataHandle(hWindows);
		GetDDEConn()->Display(ExplainError());
		
		if(csWindows.IsEmpty() == FALSE)	
		{
			csWindows += " ";
		}
	}
	
	GetDDEConn()->Display(csWindows + m_pstrWWWTopics[m_ListWindows] );
}
#endif

DWORD CDDEObject::WWW_Version()	
{


	DWORD dwVersion = 0;
	if ( srvMosaic == m_iServer )
	{
		CDDEConversation *pConv = ClientConnect( m_QueryVersion );
		DWORD dwMajor = 1;
		DWORD dwMinor = 0;
		// Spry format
		if(pConv != NULL)	
		{
			CString csTemp( m_pstrWWWTopics[m_Version] );
			HSZ hszItem = ClientArguments("DW,DW,QCS", &dwMajor, &dwMinor, &csTemp);
			HDDEDATA hVersion = pConv->ClientRequest(hszItem);
			if ( hVersion )
			{
				ServerReturned(hVersion, "DW", &dwVersion);
			}

			pConv->DoDisconnect();
		}
	}

	// Netscape  style
	if ( dwVersion == 0 )
	{

		CDDEConversation *pConv = ClientConnect( m_Version );
	
	 	if(pConv != NULL)	
		{
			CString csTemp( m_csServiceName );
			HSZ hszItem = ClientArguments("CS", &csTemp);
			HDDEDATA hVersion = pConv->ClientRequest(hszItem);
			if ( hVersion )
			{
				ServerReturned(hVersion, "DW", &dwVersion);
			}

			pConv->DoDisconnect();
		}
	}

	return dwVersion;
		
}

#if 0
void CDDEObject::WWW_GetWindowInfo(DWORD dwWindowID, CString& csURL, CString& csTitle )	
{
	CDDEConversation *pConv = ClientConnect(m_GetWindowInfo);
	
	if(pConv != NULL)	
	{
		HSZ hszItem = ClientArguments("DW", &dwWindowID);
		HDDEDATA hWinInfo = pConv->ClientRequest(hszItem);
		
		if(hWinInfo != NULL)	
		{
			ServerReturned(hWinInfo, "QCS,QCS", &csURL, &csTitle);
		}

		pConv->DoDisconnect();
	}
	
}
#endif

DWORD  CDDEObject::WWW_OpenURL(CString csURL, CString csSaveAs, DWORD dwWindowID,
				DWORD dwFlags, CString csPostFormData, CString csPostMIMEType,
				CString csProgressServer)	
{
	CDDEConversation *pConv = ClientConnect(m_OpenURL);
	DWORD dwServicingID = 0;
	
	if(pConv != NULL)	
	{

		HSZ hszItem;
		if ( srvMosaic != m_iServer )
		{
			// Careful to pass optional parameters as NULL here.
			hszItem = ClientArguments("QCS,QCS,DW,DW,QCS,QCS,CS",
				&csURL,
				csSaveAs.IsEmpty() ? NULL : &csSaveAs,
				&dwWindowID,
				&dwFlags,
				csPostFormData.IsEmpty() ? NULL : &csPostFormData,
				csPostMIMEType.IsEmpty() ? NULL : &csPostMIMEType,
				csProgressServer.IsEmpty() ? NULL : &csProgressServer);
		}
		else
		{
			#if 0
			// Careful to pass optional parameters as NULL here.
			hszItem = ClientArguments("QCS,QCS,DW,DW,QCS,QCS,QCS,QCS",
				&csURL,
				csSaveAs.IsEmpty() ? NULL : &csSaveAs,
				&dwWindowID,
				&dwFlags,
				csPostFormData.IsEmpty() ? NULL : &csPostFormData,
				csPostMIMEType.IsEmpty() ? NULL : &csPostMIMEType,
				csProgressServer.IsEmpty() ? NULL : &csProgressServer,
				csProgressServer.IsEmpty() ? NULL : &csProgressServer );
			#else
			// Careful to pass optional parameters as NULL here.
			hszItem = ClientArguments("QCS,QCS,DW,DW,QCS,QCS,QCS,QCS",
				&csURL,
				csSaveAs.IsEmpty() ? NULL : &csSaveAs,
				&dwWindowID,
				&dwFlags,
				csPostFormData.IsEmpty() ? NULL : &csPostFormData,
				csPostMIMEType.IsEmpty() ? NULL : &csPostMIMEType,
				csProgressServer.IsEmpty() ? NULL : &csProgressServer,
				NULL  );
			#endif
		}

		HDDEDATA hServiceID = pConv->ClientRequest(hszItem);
		
		if(hServiceID != NULL)	
		{
			ServerReturned(hServiceID, "DW", &dwServicingID);
		}

		pConv->DoDisconnect();
	}

	return 	dwServicingID;

}

void CDDEObject::WWW_ParseAnchor(CString csAbsolute, CString csRelative, CString& csCombinedURL)	
{
	CDDEConversation *pConv = ClientConnect(m_ParseAnchor);

	csCombinedURL.Empty();
	
	if(pConv != NULL)	
	{
		HSZ hszItem = ClientArguments("QCS,QCS", &csAbsolute, &csRelative);
		HDDEDATA hCombined = pConv->ClientRequest(hszItem);
		
		if(hCombined != NULL)	
		{
			ServerReturned(hCombined, "QCS", &csCombinedURL);
		}
		
		pConv->DoDisconnect();
	}
	
}

void CDDEObject::WWW_QueryURLFile(CString csFileName, CString& csURL)	
{
	CDDEConversation *pConv = ClientConnect(m_QueryURLFile);
	
	csURL.Empty();
	
	if(pConv != NULL)	
	{
		HSZ hszItem = ClientArguments("QCS", &csFileName);
		HDDEDATA hURL = pConv->ClientRequest(hszItem);
		
		if(hURL != NULL)	
		{
			ServerReturned(hURL, "QCS", &csURL);
		}
		
		pConv->DoDisconnect();
	}
}

#if 0
bool CDDEObject::WWW_RegisterProtocol(CString csServer, CString csProtocol)	
{
	CDDEConversation *pConv = ClientConnect(m_RegisterProtocol);
	TwoByteBool bRegistered = FALSE;
	
	if(pConv != NULL)	
	{
		HSZ hszItem = ClientArguments("QCS,QCS", &csServer, &csProtocol);
		HDDEDATA hRegistered = pConv->ClientRequest(hszItem);
		
		if(hRegistered != NULL)	{
			ServerReturned(hRegistered, "BL", &bRegistered);
		}
		
		pConv->DoDisconnect();
	}

	return (bool)bRegistered;
}

void CDDEObject::WWW_RegisterURLEcho(CString csServer)	
{
	CDDEConversation *pConv = ClientConnect(m_RegisterURLEcho);
	HDDEDATA hStatus = DDE_FNOTPROCESSED;
	
	if(pConv != NULL)	
	{
		HSZ hszItem = ClientArguments("QCS", &csServer);
		hStatus = pConv->ClientPoke(hszItem);
		
		pConv->DoDisconnect();
	}
	
}
#endif

bool CDDEObject::WWW_RegisterViewer(CString csServer, CString csMIMEType,
												DWORD dwFlags)	
{

	CDDEConversation *pConv = ClientConnect(m_RegisterViewer);
	TwoByteBool bRegistered = FALSE;
	
	if(pConv != NULL)	
	{
		HSZ hszItem = ClientArguments("QCS,QCS,DW", &csServer, &csMIMEType, &dwFlags);
		HDDEDATA hRegistered = pConv->ClientRequest(hszItem);
		
		if(hRegistered != NULL)	
		{
			ServerReturned(hRegistered, "BL", &bRegistered);
		}
		
		pConv->DoDisconnect();
	}

	return (bRegistered != FALSE);
}

#if 0

DWORD CDDEObject::WWW_RegisterWindowChange(CString csServer, DWORD dwWindowID)
{
	CDDEConversation *pConv = ClientConnect(m_RegisterWindowChange);
	DWORD dwMonitorID = 0;
	
	if(pConv != NULL)	{
		HSZ hszItem = ClientArguments("QCS,DW", &csServer, &dwWindowID);
		HDDEDATA hMonitoring = pConv->ClientRequest(hszItem);
		
		if(hMonitoring != NULL)	{
			ServerReturned(hMonitoring, "DW", &dwMonitorID);
		}
		
		pConv->DoDisconnect();
	}

	return 	dwMonitorID;
	
}
#endif

DWORD CDDEObject::WWW_ShowFile(CString csFileName, CString csMimeType,
									DWORD dwWindowID, CString csURL)	
{
	CDDEConversation *pConv = ClientConnect(m_ShowFile);
	DWORD dwServicing = 0;
	
	if(pConv != NULL)	
	{
		HSZ hszItem;
		if ( srvMosaic == m_iServer )
		{
			hszItem = ClientArguments("QCS,QCS,DW,QCS,QCS", &csFileName, &csMimeType,
				&dwWindowID, &csURL, NULL);
		}
		else
		{
			hszItem = ClientArguments("QCS,QCS,DW,QCS", &csFileName, &csMimeType,
				&dwWindowID, &csURL);
		}
		HDDEDATA hServicing = pConv->ClientRequest(hszItem);
		
		if(hServicing != NULL)	
		{
			ServerReturned(hServicing, "DW", &dwServicing);
		}
		
		pConv->DoDisconnect();
	}
	
	return 	dwServicing;
}

#if 0
bool CDDEObject::WWW_UnRegisterProtocol(CString csServer, CString csProtocol)	
{
	CDDEConversation *pConv = ClientConnect(m_UnRegisterProtocol);
	TwoByteBool bUnRegistered = FALSE;
	
	if(pConv != NULL)	{
		HSZ hszItem = ClientArguments("QCS,QCS", &csServer, &csProtocol);
		HDDEDATA hUnRegistered = pConv->ClientRequest(hszItem);
		
		if(hUnRegistered != NULL)	
		{
			ServerReturned(hUnRegistered, "BL", &bUnRegistered);
		}
		
		pConv->DoDisconnect();
	}
	
	return (bool)bUnRegistered;	
}

void CDDEObject::WWW_UnRegisterURLEcho(CString csServer)	
{
	CDDEConversation *pConv = ClientConnect(m_UnRegisterURLEcho);
	HDDEDATA hStatus = DDE_FNOTPROCESSED;
	
	if(pConv != NULL)	
	{
		HSZ hszItem = ClientArguments("QCS", &csServer);
		hStatus = pConv->ClientPoke(hszItem);
		
		pConv->DoDisconnect();
	}
	
}
#endif

bool CDDEObject::WWW_UnRegisterViewer(CString csServer, CString csMIMEType)	
{
	CDDEConversation *pConv = ClientConnect(m_UnRegisterViewer);
	TwoByteBool bUnRegistered = FALSE;
	
	if(pConv != NULL)	
	{
		HSZ hszItem = ClientArguments("QCS,QCS", &csServer, &csMIMEType);
		HDDEDATA hUnRegistered = pConv->ClientRequest(hszItem);
		
		if(hUnRegistered != NULL)	
		{
			ServerReturned(hUnRegistered, "BL", &bUnRegistered);
		}
		
		pConv->DoDisconnect();
	}
	
	return (bUnRegistered != FALSE);	
}
#if 0
bool CDDEObject::WWW_UnRegisterWindowChange(CString csServer, DWORD dwWindowID)	
{
	CDDEConversation *pConv = ClientConnect(m_UnRegisterWindowChange);
	TwoByteBool bUnRegistered = FALSE;
	
	if(pConv != NULL)	
	{
		HSZ hszItem = ClientArguments("QCS,DW", &csServer, &dwWindowID);
		HDDEDATA hUnRegistered = pConv->ClientRequest(hszItem);
		
		if(hUnRegistered != NULL)	
		{
			ServerReturned(hUnRegistered, "BL", &bUnRegistered);
		}
		
		pConv->DoDisconnect();
	}
	
	return (bool)bUnRegistered;	
}

void CDDEObject::WWW_WindowChange(DWORD dwWindowID, DWORD dwWindowFlags,
	DWORD dwX, DWORD dwY, DWORD dwCX, DWORD dwCY)	
{
	CDDEConversation *pConv = ClientConnect(m_WindowChange);
	HDDEDATA hStatus = DDE_FNOTPROCESSED;
	
	if(pConv != NULL)	
	{
		//	Be careful here to check optional arguments.
		HSZ hszItem = ClientArguments("DW,DW,DW,DW,DW,DW",
			&dwWindowID,
			&dwWindowFlags,
			(dwWindowFlags & 1) ? &dwX : NULL,
			(dwWindowFlags & 1) ? &dwY : NULL,
			(dwWindowFlags & 1) ? &dwCX : NULL,
			(dwWindowFlags & 1) ? &dwCY : NULL);
		hStatus = pConv->ClientPoke(hszItem);
		
		pConv->DoDisconnect();
	}
	
	char aBuf[512];
	sprintf(aBuf, "WWW_WindowChange(%lu,%lu,%lu,%lu,%lu,%lu);",
		dwWindowID, dwWindowFlags, dwX, dwY, dwCX, dwCY);
	GetDDEConn()->Display(aBuf);
}
#endif

//	Persistance
void CDDEObject::Serialize(CArchive& ar)	
{
	if(ar.IsStoring())	
	{
		//	Add storing code.
	}
	else	
	{
		//	Add loading code.
	}
	
	//	Serialize the base.
	CObject::Serialize(ar);
}

#ifdef _DEBUG

void CDDEObject::AssertValid() const	
{
	//	Call base to AssertValid.
	CObject::AssertValid();
	
	//	Check our instance specific members.
	ASSERT(m_pDDEConn);	//	Must always have a doc assigned!
	ASSERT(!m_csServiceName.IsEmpty());	//	Must always have a DDE server name!
	ASSERT(m_rIndex);	//	Position in list
}

void CDDEObject::Dump(CDumpContext& dc)	const 
{
	//	Have the base dump.
	CObject::Dump(dc);
	
	//	Now we dump.
	dc << "Service name: " << m_csServiceName << "\n";
}

#endif // _DEBUG

//	DDEML Callback function
#ifdef _WIN32
HDDEDATA CALLBACK ChacoVRMLDdeCallback(UINT type, UINT fmt,
	HCONV hconv, HSZ hsz1, HSZ hsz2, HDDEDATA hData, DWORD dwData1,
	DWORD dwData2)	
{
#else
HDDEDATA CALLBACK _export ChacoVRMLDdeCallback(UINT type, UINT fmt,
	HCONV hconv, HSZ hsz1, HSZ hsz2, HDDEDATA hData, DWORD dwData1,
	DWORD dwData2)	
{
#endif // _WIN32
	
	switch(type)	
	{				//	ReturnType			Receiver	
		case XTYP_CONNECT:	
		{			//	XCLASS_BOOL			server
			//	Find the object with the registered service name.
			CDDEObject *pObj = CDDEObject::ResolveService(hsz2);
			if(pObj != NULL)	
			{
				//	Leave it up to the document to decide wether or not it wants the
				//		conversation.
				return(pObj->ServerConnect(hsz1));
			}
			return(DDEFALSE);
		}
	
		case XTYP_CONNECT_CONFIRM:	
		{	//	XCLASS_NOTIFICATION	server
			//	Find the object with the registered service name.
			CDDEObject *pObj = CDDEObject::ResolveService(hsz2);
			if(pObj != NULL)	{
				//	The object has already accepted a conversation for the given
				//		topic, so no need to check that.  Let it know it has a real
				//		conversation now, and give it the handle.
				pObj->ServerConnectConfirm(hconv);
			}
			return(NOTHING);
		}
	
		case XTYP_DISCONNECT:	
		{		//	XCLASS_NOTIFICATION	client/server
			//	Find the object handling the conversation.
			CDDEConversation *pConv = CDDEConversation::ResolveConversation(hconv);
			if(pConv != NULL)	{
				//	We also need to remove the conversation from the list of
				//		conversations managed by some DDE service object.
				CDDEObject::DDEDisconnect(pConv);
		
				//	It's over.
				delete pConv;
			}
			return(NOTHING);
		}
	
		case XTYP_POKE:	
		{				//	XCLASS_FLAGS		server
			//	Find the object handling the conversation.
			CDDEConversation *pConv = CDDEConversation::ResolveConversation(hconv);
			if(pConv != NULL)	
			{
				//	Have the object handle the details.
				return(pConv->ServerPoke(hsz1, hsz2, hData));
			}
			return(DDE_FNOTPROCESSED);
		}
	
		case XTYP_REQUEST:	
		{			//	XCLASS_DATA			server
			//	Find the object handling the conversation.
			CDDEConversation *pConv = CDDEConversation::ResolveConversation(hconv);
			if(pConv != NULL)	
			{
				return(pConv->ServerRequest(hsz1, hsz2));
			}
			return(NULL);
		}
	}	//	end switch
	
	//	Not handled.
	return(NOTHING);
}
