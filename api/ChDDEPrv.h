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


----------------------------------------------------------------------------*/

// $Header$

#if defined( CH_VRML_VIEWER )
//#define DDE_SERVICE_NAME		"Chaco_DDE_VRML_Viewer"
#define DDE_SERVICE_NAME		"Chaco_VRScout"
#else
#define DDE_SERVICE_NAME		"Chaco_Pueblo_Client"
#endif


#define DDETIMEOUT 60000UL					//	One minute in milliseconds

#define NOTHING ((HDDEDATA)NULL)
#define DDETRUE ((HDDEDATA)TRUE)
#define DDEFALSE ((HDDEDATA)FALSE)
#define DDEFACK ((HDDEDATA)DDE_FACK)

//	If compiling in 32 bit mode, BOOLs change size.
typedef short TwoByteBool;


#ifdef _WIN32
HDDEDATA CALLBACK ChacoVRMLDdeCallback(UINT type, UINT fmt,
	HCONV hconv, HSZ hsz1, HSZ hsz2, HDDEDATA hData, DWORD dwData1,
	DWORD dwData2);
#else
HDDEDATA CALLBACK _export ChacoVRMLDdeCallback(UINT type, UINT fmt,
	HCONV hconv, HSZ hsz1, HSZ hsz2, HDDEDATA hData, DWORD dwData1,
	DWORD dwData2);

#endif // _WIN32

char *SkipToNextArgument(char *pFormat);
char *ExtractArgument(HSZ hszArguments, int iWhichArg);
CString ExplainPoke(HDDEDATA hStatus);
CString ExplainBool(TwoByteBool bFlag);
CString ExplainError();


//	Forward declarations
class CDDEConversation;

//	Dependent headers, never included standalone....

class CDDEObject : public CObject	
{
	public:
		virtual void Serialize(CArchive& ar);
		DECLARE_SERIAL( CDDEObject )
	
	#ifdef _DEBUG
		virtual void AssertValid() const;
		virtual void Dump(CDumpContext& dc) const;
	#endif // _DEBUG

		CDDEObject( int iServer, ChHTTPDDE *pDDEConn );
		~CDDEObject();
		ChHTTPDDE * GetDDEConn()	{ return m_pDDEConn; }
		int		    GetServerType() { return m_iServer; }
		const char*	GetServerName( int iServer )	
							{ 
								ASSERT( iServer < maxServers );
								return m_pstrServers[iServer];
							}
	
		void   		SetServer( int iServer );
		static CDDEObject *ResolveService(const char *pServiceName);
		static CDDEObject *ResolveService(HSZ hszServiceName);
		static CDDEObject *ResolveConversation(CDDEConversation *pConv);
	
		HDDEDATA ServerConnect(HSZ hszTopic);
		void ServerConnectConfirm(HCONV hConv);
		static void DDEDisconnect(CDDEConversation *pConv);

		//	The DDEML instance identifier.
		static DWORD m_dwidInst;
	
		CString GetServiceName()	{ return m_csServiceName; }
		HSZ GetServiceHSZ()	{ return m_hszServiceName; }
		void SetRegNowID( DWORD dwID )	{ m_dwRegTransactionID = dwID; }


		enum tagServers {
							srvMosaic = 0,			// NCSA
							srvWEBSURFER, 		// Netmanage 
							srvNETSCAPE,		// Netscape
							srvIExplore,		// MS Internet explorer
							maxServers
						};

		//	Possible topics.
		enum	{
			m_Activate = 0,
			m_Alert,
			m_BeginProgress,
			m_CancelProgress,
			m_EndProgress,
			//m_Exit,
			//m_GetWindowInfo,
			//m_ListWindows,
			m_MakingProgress,
			m_OpenURL,
			m_ParseAnchor,
			m_QueryURLFile,
			m_QueryViewer,
			//m_RegisterProtocol,
			//m_RegisterURLEcho,
			m_RegisterViewer,
			//m_RegisterWindowChange,
			m_SetProgressRange,
			m_ShowFile,
			//m_UnRegisterProtocol,
			//m_UnRegisterURLEcho,
			m_UnRegisterViewer,
			//m_UnRegisterWindowChange,
			//m_URLEcho,
			m_Version,
			m_ViewDocFile,
			//m_WindowChange,
			m_QueryVersion,
			m_OpenURLResult,
			m_CancelTransaction,
			//m_ViewDocCache,	
			m_RegisterNow,
			m_RegisterDone,
			m_MaxTopics
		};
		static int EnumTopic(HSZ hszTopic);
	protected:
		//	Dynamic creation.
		CDDEObject();
	
		//	List of currently running CDDEObjects, and our position in the list.
		static CObList *m_pcolRunning;
		POSITION m_rIndex;

		//	Used to generate unique DDE services on a per instance basis.
		CString m_csServiceName;
		HSZ m_hszServiceName;
	
		//	The document owning us.
		ChHTTPDDE	 		*m_pDDEConn;
		int			  		m_iServer;
		DWORD		 		m_dwRegTransactionID;
		//	The list of DDE object that this service is handling.
		CObList m_colConversations;
	
		static HSZ m_aTopics[m_MaxTopics];
		static const char* m_pstrServers[maxServers];
		void LoadTopics();
		void FlushTopics();
	
		//	Client/Server useful methods.
		CDDEConversation *ClientConnect(int iTopic);
		HSZ ClientArguments(const char *pFormat, ...);
		void ServerReturned(HDDEDATA hData, const char *pFormat, ...);
	
		//	Client browser interface.
		HSZ m_hszBrowser;
	public:
		DWORD 	WWW_Activate(DWORD dwWindowID, DWORD dwFlags = 0ul);
		void 	WWW_CancelProgress(DWORD dwTransactionID);
		//void 	WWW_Exit();
		//void 	WWW_ListWindows();
		DWORD 	WWW_Version();
		//void 	WWW_GetWindowInfo(DWORD dwWindowID,  CString& csURL, CString& csTitle );
		DWORD 	WWW_OpenURL(CString csURL, CString csSaveAs, DWORD dwWindowID,
					DWORD dwFlags, CString csPostFormData, CString csPostMIMEType,
					CString csProgressServer);
		void 	WWW_ParseAnchor(CString csAbsolute, CString csRelative, CString& csCombinedURL );
		void 	WWW_QueryURLFile(CString csFileName, CString& csURL);
		//bool 	WWW_RegisterProtocol(CString csServer, CString csProtocol);
		//void 	WWW_RegisterURLEcho(CString csServer);
		bool 	WWW_RegisterViewer(CString csServer, CString csMIMEType, DWORD dwFlags);
		//DWORD 	WWW_RegisterWindowChange(CString csServer, DWORD dwWindowID);
		DWORD 	WWW_ShowFile(CString csFileName, CString csMimeType, DWORD dwWindowID,
										CString csURL);
		//bool 	WWW_UnRegisterProtocol(CString csServer, CString csProtocol);
		//void 	WWW_UnRegisterURLEcho(CString csServer);
		bool 	WWW_UnRegisterViewer(CString csServer, CString csMIMEType);
		//bool 	WWW_UnRegisterWindowChange(CString csServer, DWORD dwWindowID);
		//void	WWW_WindowChange(DWORD dwWindowID, DWORD dwWindowFlags,
		//				DWORD dwX, DWORD dwY, DWORD dwCX, DWORD dwCY);
		void 	WWW_RegisterDone( );

	
	private:
		//	Common construction code.
		void CommonConstruction( int iServer );
		static char* m_pstrWWWTopics[m_MaxTopics];


};



class CDDEConversation : public CObject	
{
	public:
		virtual void Serialize(CArchive& ar);
		DECLARE_SERIAL( CDDEConversation )
	
	#ifdef _DEBUG
		virtual void AssertValid() const;
		virtual void Dump(CDumpContext& dc) const;
	#endif // _DEBUG

		CDDEConversation(HCONV hConv);
		~CDDEConversation();
		ChHTTPDDE *GetDDEConn();
	
		static CDDEConversation *ResolveConversation(HCONV hConv);
	
		void DoDisconnect();
	
		HDDEDATA ServerPoke(HSZ hszTopic, HSZ hszItem, HDDEDATA hDataPoke);
		HDDEDATA ServerRequest(HSZ hszTopic, HSZ hszItem);	
		HDDEDATA ClientRequest(HSZ hszItem);
		HDDEDATA ClientPoke(HSZ hszItem);
	
	protected:
		//	Conversation that we're handling.
		HCONV m_hConv;
	
		//	List of all conversations going on (for informational lookup), and instance
		//		index into that list.
		static CObList *m_pcolConversations;
		POSITION m_rIndex;
	
		CDDEConversation();
	
		//	Useful routines for client/server interaction.
		void ClientPassed(HSZ hszItem, const char *pFormat, ...);
		HDDEDATA ServerReturns( HSZ hszItem, const char *pFormat, ...);
	
		//	Server Pokes
		HDDEDATA WWW_EndProgress(HSZ hszItem);
		HDDEDATA WWW_SetProgressRange(HSZ hszItem);
		//HDDEDATA WWW_URLEcho(HSZ hszItem);
		HDDEDATA WWW_ViewDocFile(HSZ hszItem);
		//HDDEDATA WWW_WindowChange(HSZ hszItem);
		HDDEDATA WWW_OpenURLResult(HSZ hszItem);
	
		//	Server Requests
		HDDEDATA WWW_Alert(HSZ hszItem);
		HDDEDATA WWW_BeginProgress(HSZ hszItem);
		HDDEDATA WWW_MakingProgress(HSZ hszItem);
		HDDEDATA WWW_OpenURL(HSZ hszItem);
		HDDEDATA WWW_QueryViewer(HSZ hszItem);
		//HDDEDATA WWW_ViewDocCache(HSZ hszItem);
		HDDEDATA WWW_RegisterNow(HSZ hszItem);
	
	private:
		void CommonConstruction();
};

// $Log$
