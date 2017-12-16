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

	This file contains the definition of the ChCoreStreamManager class.

----------------------------------------------------------------------------*/

#if (!defined( CHCORESTREAM_H ))
#define CHCORESTREAM_H
#include <ChHttpStream.h>

class ChClientCore;

/*----------------------------------------------------------------------------
	ChCoreStreamManager class
----------------------------------------------------------------------------*/
class ChCoreHTTPReq
{
	public :
			enum RequestType { coreReq = 1, moduleReq, startReq, quitReq,
									registerReq };
	public :
		ChCoreHTTPReq( int iReqID );
		virtual ~ChCoreHTTPReq( )		{}

		inline int 			 GetReqID( )					{ return m_iReqID; }
		virtual int GetType( )								{ return coreReq; }

	private :
		int						m_iReqID;
};

class ChCoreModuleReq : public ChCoreHTTPReq
{
	public :
		ChCoreModuleReq( int iReqID, ChHTTPStreamManager* pDstStream, chparam userData  );
		virtual ~ChCoreModuleReq( )		{}

		virtual int GetType( )										{ return moduleReq; }
		inline ChHTTPStreamManager* GetModuleStream( )				{ return m_pModuleStream; }
		inline chparam				GetModuleUserData( )			{ return m_reqUserData; }

	private :
		ChHTTPStreamManager* 	m_pModuleStream;
		chparam					m_reqUserData;
};

class ChCoreStartReq : public ChCoreHTTPReq
{
	public :
		ChCoreStartReq( int iReqID );
		virtual ~ChCoreStartReq( )		{}

		virtual int GetType( )										{ return startReq; }

		void 	OnStartRequestComplete( ChClientCore* pCore, int iReason, const char* pstrFile );
	private :
};

class ChCoreQuitReq : public ChCoreHTTPReq
{
	public :
		ChCoreQuitReq( int iReqID );
		virtual ~ChCoreQuitReq( )		{}

		virtual int GetType( )										{ return quitReq; }

		void 	OnQuitRequestComplete( ChClientCore* pCore, int iReason, const char* pstrFile );
	private :
};

class ChCoreRegisterReq : public ChCoreHTTPReq
{
	public :
		ChCoreRegisterReq( int iReqID, const ChString& strRegInfo );
		virtual ~ChCoreRegisterReq( )		{}

		virtual int GetType( )										{ return registerReq; }

		void 	OnRegisterRequestComplete( ChClientCore* pCore, int iReason, const char* pstrFile  );
	private :
		ChString  m_strRegInfo;
};






/*----------------------------------------------------------------------------
	ChCoreStreamManager class
----------------------------------------------------------------------------*/


class ChCoreStreamManager  :  public  ChHTTPStreamManager
{
	public :
		ChCoreStreamManager( ChClientCore* pCore ) : m_pCore( pCore )
						{
						}
		virtual ~ChCoreStreamManager() 		
						{
						}

	    virtual int NewStream( chparam requestData, pChHTTPStream pStream, bool boolSeekable  );
	    virtual void DestroyStream( chparam requestData, pChHTTPStream pStream, int iReason );
	    virtual chint32 WriteReady( chparam requestData, pChHTTPStream pStream, chint32 iBytes );
	    virtual chint32 Write( chparam requestData, pChHTTPStream pStream, 
	    								chint32 lOffset, chint32 lLen, const char* pBuffer );
	 	virtual void StreamAsFile(chparam requestData, pChHTTPStream pStream, const char* fname);

	private :
		inline ChClientCore* GetCore() 		{ return m_pCore; }
		ChClientCore*		m_pCore;

};




#endif // CHCORESTREAM_H
