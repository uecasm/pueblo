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

	This file consists of interfaces used by the Pueblo module manager core.  This
	file is only used on the client.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( CH_PBMODULEMGR_H_ )

#define CH_PBMODULEMGR_H_

#include <ChHtpCon.h>
#include "ChRunTimeInfo.h"
#include "ChCoreStream.h" 

class 	ChClientCore;
class  	ChModuleInfo;
class  	ChModuleRunInfo;
class  	ChRequestPending;
class 	ChDispatcher;
class   ChArgumentList;


typedef ChSplay< ChModuleID, ChString> 				ChPuebloModuleIDMap;
typedef ChSplay< ChModuleID, ChModuleRunInfo*> 		ChModuleRunTable;
typedef ChSplay< ChString, ChRequestPending*> 		ChModulePendingList;

class ChModuleInfo
{
	public :
		ChModuleInfo( ) :
						m_idModule( 0 )
						{
						}
		ChModuleInfo( ChModuleID idModule,  const ChString& strLibrary ) :
										m_idModule( idModule ),
										m_strLibrary( strLibrary )
									{
									}

		ChModuleInfo( const ChModuleInfo& modInfo ) 
								{
											m_idModule =  modInfo.GetModuleID();
											m_strLibrary = modInfo.GetLibraryName();
								}
		~ChModuleInfo( ) {}

		const ChModuleInfo& operator=( const ChModuleInfo& modInfo )
					{
						m_idModule = modInfo.GetModuleID();	
						m_strLibrary = modInfo.GetLibraryName();
						return *this;
					}

		ChModuleID  	GetModuleID() const			{ return m_idModule; }
		const ChString&  	GetLibraryName() const		{ return m_strLibrary; }

	private :
		ChModuleID		m_idModule;
		ChString 			m_strLibrary;
};

typedef ChSplay< ChString, ChModuleInfo> 			ChPuebloModuleList;


class ChRequestPending
{
	public :
		ChRequestPending( const ChString& strModule );
		~ChRequestPending( );

		const ChString&  	GetModuleName()		{ return m_strModule; }
		int 		   	NumPending()		{ return m_iNumPending; }
		void 			IncrementPending()	{ m_iNumPending++; }	
	private :
		int 			m_iNumPending;
		ChString 			m_strModule;
};



class ChPuebloModuleManager
{
	public :
		ChPuebloModuleManager( ChClientCore* pCore );
		~ChPuebloModuleManager();

		ChModuleID GetModuleID( const ChString& strModule );
		ChString GetModuleName( const ChModuleID idModule );
		ChModuleRunInfo* GetRunInfo( const ChModuleID idModule );
		ChDispatcher* GetDispatcher( const ChModuleID idModule );
		int	GetModuleCount();
		int GetModuleIDs( int iModuleCount, ChModuleID* pModules );

		void HideAll();

		void UnloadModule( const ChModuleID idModule );
		void UnloadAll();

		ChHTTPSocketConn* GetHTTPConnection() { return &m_httpConnection; }

		bool OnStartup();
		bool OnShutDown();
		void OnSecondTick( time_t timeCurr );

		bool OnLoadModule( const ChString& strModule, ChArgumentList *pArgList );

		const ChString&  GetAppDirectory() { return m_strAppDir; }

	private :

		void InitModuleManager();
		bool InitCore();
		void BuildModuleList();

 		bool FindModule( const ChString& strModuleName, ChString& strLibrary );
		bool LoadPuebloModule( const ChString& strModule, const ChString& strLibrary, ChArgumentList *pArgList );

	private : 
		ChClientCore*				m_pCore;
		ChHTTPSocketConn			m_httpConnection;
		ChCoreStreamManager			m_coreStreamManager;
		ChModuleRunTable			m_moduleRunTable;
		ChModulePendingList			m_pendingReqTable;
		ChModuleID					m_idGenerator;
		bool						m_boolInUnloadAll;

		// There is only one module list and a ID map table for one APP

		static ChPuebloModuleList	m_moduleList;
		static ChPuebloModuleIDMap	m_moduleIDMap;
		static ChString				m_strAppDir;
};


#endif // CH_PBMODULEMGR_H_

// $Log$
