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

	This file consists of the Chaco DDE interface to WEB clients.

----------------------------------------------------------------------------*/

#if (!defined( _CHDDE_H ))
#define _CHDDE_H


#include <ddeml.h>
#include <ChSplay.h> 
#include <ChList.h>
#include <ChHTTP.h>



// Private classes used by DDE class, the interface is defined in ChDDE.cpp 
class ChDDEConnInfo;
class ChHTTPConvInfo;
class CDDEObject;
class ChVisitedInfo;

typedef ChVisitedInfo *pChVisitedInfo;
typedef ChDDEConnInfo *pChDDEConnInfo;



typedef ChSplay< ChString, pChVisitedInfo > ChVisitedList;            
typedef ChPtrList<ChDDEConnInfo > 	ChConnActiveList;            
typedef ChPtrList<ChDDEConnInfo>	ChConnWaitList;     





class CH_EXPORT_CLASS ChHTTPDDE : public ChHTTPConn
{

	public :
		ChHTTPDDE( ChHTTPStreamManager* pStreamMgr, chuint32 flOptions = 0 );

		virtual ~ChHTTPDDE();

		// Methods
		virtual int			GetConnectionType()				
							{ 
								return  usingDDE; 
							}

		virtual bool GetURL( const ChString& strURL, chparam userData,
						const char* pstrDefURL = 0,
						chuint32 flOptions = 0, ChHTTPStreamManager* pStreamMgr = 0 );

		virtual bool PostURL( const ChString& strURL, const char* pstrBody,
							chint32 lLen = -1, chparam userData = 0,
							const char* pstrDefURL = 0, 
							chuint32 flOptions = 0, ChHTTPStreamManager* pStreamMgr = 0 )
							{	 
								return FALSE;
							}
		virtual void QueryURL( const ChString& strFile, ChString& strURL, ChString& strMimeType );
		virtual bool IsURLCached( const ChString& strURL, const char* pstrDefURL = 0 )
						{
							return false;
						}
		virtual bool IsVisitedURL( const ChString& strURL, const char* pstrDefURL = 0 )
						{
							return false;
						}
		virtual bool GetCachedURL( const ChString& strURL, ChString& strName, 
								ChString& strMimeType, const char* pstrDefURL  = 0 , 
								chuint32 flOption  = 0 )
						{
							return false;
						}
		virtual bool DeleteCachedURL( const ChString& pstrURL, const char* pstrDefURL = 0 )
						{
							return false;
						}
		virtual void AbortRequests( bool boolAbortPrefetch = false, ChHTTPStreamManager* pStreamMgr = 0 );	 

		virtual bool ViewFile( const ChString strURL, const ChString& strFile ); 
	

		virtual bool IsActive();
		virtual int  NumActive();
		virtual void GetProgressMsg( ChString& strMsg, int& iPercentComplete ); 

		

		virtual void ShutDownHTTP();

	public:
		
		static ChVisitedList& GetVisitedList()			{ return m_visitedList; }
		static ChConnActiveList& GetConnectionList()	{ return m_connActiveList; }
		static ChConnWaitList& GetWaitList()			{ return m_connWaitList; }

	public :
		// DDE related methods
		void ViewDocFile( CString& strFile, CString&strURL, CString& strMimeType );
		DWORD Alert( const CString& csMessage, DWORD dwType, DWORD dwButtons );
		void SetProgressRange( DWORD dwID, DWORD dwRange );
		void BeginProcess( DWORD dwTransactionID,  const CString& csInitialMessage );
		bool MakingProgress( DWORD dwTransactionID, const CString& csMessage, DWORD dwProgress );
		void EndProgress( DWORD dwProgress );
		void OpenURLResult( DWORD dwTrans, DWORD dwWindow );
		void RegisterNow();
		virtual bool ProcessPendingRequest(  UINT uRequest  );

		HWND GetDDEPostingWindow( );

	
		void Display(CString strMsg)
				{
					//TRACE( strMsg );
				}


	private :
		void InitDDEConn();
		BOOL RegisterViewers( );
		BOOL UnregisterViewers( );
		bool EstablishConversation();
		bool IsConnectionValid()		{ return m_boolConnectionValid; }
		bool ProcessConnectionRequest( );
		


	private :
		// Data members
		CString			m_strServiceName;

		CDDEObject 			*m_pDDE;
		bool				m_boolStopLoading;
		bool				m_boolConnectionValid;
		bool				m_boolNotifyDone;
		DWORD				m_dwTransactionID;
		DWORD				m_dwWindowID;
		HWND				m_hWndDDE;

		static ChVisitedList	m_visitedList;
		static ChConnActiveList	m_connActiveList;
		static ChConnWaitList	m_connWaitList;
		static unsigned	long	m_numObjects;
		static ChString			m_strStatusMsg;
		static DWORD			m_dwProgress;
		static DWORD			m_dwMaxRange;

};
#endif // defined( CH_USE_DDE )
