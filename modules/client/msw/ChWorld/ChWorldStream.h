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

#if !defined( _CHWORLDSTRM_H )
#define _CHWORLDSTRM_H

#include <ChHttpStream.h>
#include <ChPuebloScript.h>

class ChWorldMainInfo;


class ChWorldScript : public ChPuebloScript
{
	public :
		ChWorldScript( ChWorldMainInfo* pInfo );
		virtual ~ChWorldScript()		{}

		
		virtual bool	ExecuteCommand( int iCommand,  ChArgumentList& attrList,
										   ChArgumentList& argList );

	private :
		ChWorldMainInfo *m_pMainInfo;	
};

/*----------------------------------------------------------------------------
	ChWorldHTTPReq class
----------------------------------------------------------------------------*/
class ChWorldHTTPReq
{
	public :
		typedef enum { typeWorldConnect, typeWorldDisconnect, typeWorldEnhanced,
						typeFile } RequestType;
	public :
		ChWorldHTTPReq( int iReqID ) : m_iReqID( iReqID )
				{
				}
		virtual ~ChWorldHTTPReq( )		{}

		inline int 			 GetReqID( )					{ return m_iReqID; }

		virtual RequestType	 GetRequestType( ) = 0;
	private :
		int						m_iReqID;
};


/*----------------------------------------------------------------------------
	ChWorldConnectHTTPReq class
----------------------------------------------------------------------------*/
class ChWorldConnectHTTPReq : public ChWorldHTTPReq
{
	public :
		ChWorldConnectHTTPReq( int iReqID ):
				ChWorldHTTPReq( iReqID )
					{
					}
		virtual ~ChWorldConnectHTTPReq(  )		{}

		virtual RequestType GetRequestType( )	{ return typeWorldConnect; }

		void 	OnConnectRequestComplete( ChWorldMainInfo* pMainInfo, int iReason, const char* pstrFile  );
	private :
};

/*----------------------------------------------------------------------------
	ChWorldDisconnectHTTPReq class
----------------------------------------------------------------------------*/
class ChWorldDisconnectHTTPReq : public ChWorldHTTPReq
{
	public :
		ChWorldDisconnectHTTPReq( int iReqID ):
				ChWorldHTTPReq( iReqID )
						{
						}
		virtual ~ChWorldDisconnectHTTPReq( )		{}

		virtual RequestType GetRequestType( )	{ return typeWorldDisconnect; }

		void 	OnDisconnectRequestComplete( ChWorldMainInfo* pMainInfo, int iReason, 
															const char* pstrFile  );
	private :
};

/*----------------------------------------------------------------------------
	ChWorldEnhancedHTTPReq class
----------------------------------------------------------------------------*/
class ChWorldEnhancedHTTPReq : public ChWorldHTTPReq
{
	public :
		ChWorldEnhancedHTTPReq( int iReqID, HWND hWnd ):
				ChWorldHTTPReq( iReqID ),
				m_hWndNotify( hWnd )
					{
					}
					
		virtual ~ChWorldEnhancedHTTPReq( )		{}
		inline const HWND GetNotifyWnd() 	 { return m_hWndNotify; }

		virtual RequestType GetRequestType( )	{ return typeWorldEnhanced; }

		void 	OnEnhancedRequestComplete( ChWorldMainInfo* pMainInfo, int iReason, 
															const char* pstrFile  );
	private :
		HWND		m_hWndNotify;
};


/*----------------------------------------------------------------------------
	ChWorldFileHTTPReq class

		A pointer to an object of this class is passed as user data when
		loading URLs.
----------------------------------------------------------------------------*/

class ChWorldFileHTTPReq   : public ChWorldHTTPReq
{

	public:
		ChWorldFileHTTPReq( int iReqID, HWND hWnd, const ChString& strHTML ) :
				ChWorldHTTPReq( iReqID ),
				m_hWndNotify( hWnd ),
				m_strHTML( strHTML )
					{
					}

		virtual RequestType GetRequestType( )	{ return typeFile; }
		inline const ChString& GetHTML() 		 { return m_strHTML; }
		inline const HWND GetNotifyWnd() 	 { return m_hWndNotify; }

		inline const ChString& GetURL() { return m_strURL; }
		inline const ChString& GetFileName() { return m_strFile; }
		inline const ChString& GetMimeType() { return m_strData; }
		inline const ChString& GetErrorMsg() { return m_strData; }  

		inline void SetLoadComplete(  const ChString& strURL, const ChString& strFile,
										const ChString& strMimeType )
						{
							m_strURL = strURL;
							m_strFile = strFile;
							m_strData = strMimeType;
						}
		inline void SetLoadError(  const ChString& strURL, const ChString& strErrMsg )
						{
							m_strURL = strURL;
							m_strData = strErrMsg;
						}
	protected:
		ChString		m_strHTML;
		HWND		m_hWndNotify;

		ChString		m_strURL;
		ChString 		m_strFile;
		ChString 		m_strData;
};


/*----------------------------------------------------------------------------
	ChWorldStreamManager class
----------------------------------------------------------------------------*/

class ChWorldStreamManager  :  public  ChHTTPStreamManager
{
	public :
		ChWorldStreamManager( ChWorldMainInfo* pMainInfo, ChModuleID idModule );
		virtual ~ChWorldStreamManager() {} 		

	    virtual int NewStream( chparam requestData, pChHTTPStream pStream,
	    						bool boolSeekable );
	    virtual void DestroyStream( chparam requestData, pChHTTPStream pStream,
	    							int iReason );
		virtual void StreamAsFile( chparam requestData, pChHTTPStream pStream,
									const char* pstrFilename );

	private :
		ChModuleID 			m_idModule;
		ChWorldMainInfo* 	m_pMainInfo;
};



#endif //!defined( _CHWORLDSTRM_H )

// $Log$
