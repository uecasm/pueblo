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

	This file contains the interface for ChCore, a base class used
	by ChServerCore and ChClientCore.

----------------------------------------------------------------------------*/

// $Header$

#if (!defined( CHCORE_H ))
#define CHCORE_H

#include <ChModule.h>
#include <ChClInfo.h>
#include <ChList.h>
#include <ChPerFrm.h>

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
#endif



/*----------------------------------------------------------------------------
	ChCore class
----------------------------------------------------------------------------*/

class ChFrameVisitor 
{
	public:
		virtual bool Visit( const ChString& strFrameName,
							ChCore* pCore ) = 0;

		virtual void Start() {};
		virtual void Stop() {};
};


/*----------------------------------------------------------------------------
	ChCore class
----------------------------------------------------------------------------*/

class ChPaneMgr;
class ChRMenuMgr;
class ChHookManager;
class ChSplitter;
class ChHTTPStreamManager;
class ChPane;
class ChHTTPConn;

class CH_EXPORT_CLASS ChCore
{
	friend class ChMainInfo;   

	public :
											/* Constants for the status bar panes
												(Used by SetStatusPaneText) */

		enum tagStatusPane { paneStatus, paneSessionTime, paneTimeofDay,
								paneProgress, paneMax };

		enum tagWebBrowser { browserWebTracker, browserExternal,
								browserUserPref };

		enum tagTraceOptions { traceHTML = 0x01, traceErrors = 0x02,
								traceWarnings = 0x04,
								traceMiscMessages = 0x8000 };

		static const chflag32		m_flTraceDefault;

	public:
		ChCore();
		virtual ~ChCore();

		inline static DWORD GetDDEInstance() { return m_dwDDEInst; }
		inline static const ChClientInfo* GetClientInfo()
				{
					return &m_clientInfo;
				}
		inline ChPaneMgr* GetPaneMgr() { return m_pPaneMgr; }

		inline void OnTraceClose()
				{
					m_pTracePane = 0;
				}

		static void  SetDDEInstance( DWORD dwInst );

		virtual void NewFrameWnd( const ChString& strArgs,
									const ChString& strLabel = "" ) const = 0;

		virtual bool ActivateFrame( const ChString& strFrameName ) = 0;

		virtual ChPersistentFrame* GetFrameWnd() const = 0;
		virtual void SetFrameTitle(	const ChString& strDocName ) = 0;
		virtual ChString GetFrameName() const = 0;
		virtual void SetFrameName( const ChString& strFrameName ) = 0; 

		inline ChPane* GetTracePane() { return m_pTracePane; }
		chflag32 GetTraceOptions() { return m_flTraceOptions; }
		void SetTraceOptions( chflag32 flTraceOptions )
				{
					m_flTraceOptions = flTraceOptions;
				}
		inline void Trace( const ChString& strText, chflag32 flType,
							bool boolHtml = false )
				{
					if (GetTracePane() && (flType & GetTraceOptions()))
					{
						DoTrace( strText, flType, boolHtml );
					}
				}

		void OpenTraceWindow();
		static void UpdateTraceOptions();

		virtual void DisplayStatus( const ChString& strStatus ) = 0;
		virtual void SetStatusPaneText( int iIndex, const ChString& strStatus ) = 0;
		virtual void UpdateSessionPane( const ChString& strPaneText, bool boolStartTracking = true ) = 0;


		virtual void EnumerateFrames( ChFrameVisitor& enumFrame ) = 0;

		virtual ChRMenuMgr *GetMenuMgr( ) = 0;
		virtual ChSplitter* GetSplitter() = 0;

		virtual bool IsFlashWindow() = 0;		
		virtual void EnableFlashWindow( bool boolFlash = true ) = 0;


		virtual ChString GetModuleName( const ChModuleID& idModule ) = 0;
		virtual void RegisterDispatcher( const ChModuleID idModule,
									ChDispatcher *pDispatcher ) = 0;
		virtual void UnregisterDispatcher( const ChModuleID idModule ) = 0;
	
		virtual bool AsyncDispatchMsg( const ChModuleID idModule, ChMsg* pMsg ) = 0;
		virtual chparam DispatchMsg( const ChModuleID idModule, ChMsg& msg ) = 0;

		virtual ChMainInfo* GetMainInfo( const ChModuleID& idModule, 
										const char* pstrFrameName = 0 ) const = 0;
		virtual ChMainInfo* GetMainInfo( const ChString& strModule, 
										const char* pstrFrameName = 0 ) const = 0;

		virtual void UnloadModule( const ChModuleID& idModule ) const = 0;
		virtual void UnloadModule( const ChString& strModule ) const = 0;

		virtual int	 GetModuleCount() = 0;
		virtual int GetModuleIDs( int iModuleCount, ChModuleID* pModules ) = 0;


		virtual ChHTTPConn* GetHTTPConn() = 0;

		virtual void AbortRequests( bool boolAbortPrefetch = false, 
										ChHTTPStreamManager* pStreamMgr = 0 ) = 0;
		virtual bool GetURL( const ChString strURL, chuint32 flOptions = 0, 
								ChHTTPStreamManager* pStream = 0,
								chparam userData = 0 ) = 0;
		virtual bool PostURL( const ChString strURL, const char* pData, chint32 lLen, 
								chuint32 flOptions = 0, 
								ChHTTPStreamManager* pStream = 0,
								chparam userData = 0 ) = 0;
		virtual void DisplayWebPage( const ChString& strURL, int iBrowser ) = 0;

		#if defined( CH_CLIENT )

		virtual void LoadClientModule( const ChString& strModuleName,
								const ChString& strModuleBase,
								const ChModuleID& idNotifyModule = 0,
								chparam userData = 0, bool boolOptional = false,
								bool boolUseExisting = true ) = 0;

		#endif	// defined( CH_CLIENT )

	protected:
		void DoTrace( const ChString& strText, chflag32 flType, bool boolHtml );

	protected:
		static ChClientInfo		m_clientInfo;
		static DWORD			m_dwDDEInst;

		ChHookManager*			m_phookCommand;
		ChHookManager*			m_phookInline;

	private:
		ChPaneMgr*				m_pPaneMgr;

		ChPane*					m_pTracePane;
		chflag32				m_flLastTraceType;
		static chflag32			m_flTraceOptions;

};


#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	#undef AFXAPP_DATA
	#define AFXAPP_DATA NEAR
#endif

#endif	// !defined( CHCORE_H )

// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
