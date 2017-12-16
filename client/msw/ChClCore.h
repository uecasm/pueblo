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

	This file consists of interfaces used by the Pueblo client core,
	including the ChClientCore class.  (One and only one of these classes
	should be created by the client core.)

----------------------------------------------------------------------------*/

#if (!defined( _CHCLCORE_H ))
#define _CHCLCORE_H

#include <ChModule.h>
#include <ChCore.h>
#include <ChMsg.h>
#include <ChConn.h>
#include <ChDispat.h>
#include <ChAcct.h>
#include <ChClInfo.h>
#include <ChArgList.h>
#if defined( CH_MSW )

	#include <ChMsgTyp.h>

#endif // CH_MSW

class ChPuebloModuleManager;
class ChHTTPSocketConn;
class ChRMenuMgr;
class ChHTTPDDE;


#define LICENSE_FILE		TEXT( "license.htm" )
#define LICENSE_URL			TEXT( "http://www.chaco.com/pueblo/text/license.html" )


/*----------------------------------------------------------------------------
	ChClientMainInfo class
----------------------------------------------------------------------------*/

#define CH_CLIENT_CORE_NAME		"Pueblo Client Core module"



/*----------------------------------------------------------------------------
	Handler declarations
----------------------------------------------------------------------------*/

CH_DECLARE_MAIN_HANDLER( coreMainHandler )
CH_DECLARE_MESSAGE_HANDLER( coreDefHandler )
CH_DECLARE_MESSAGE_HANDLER( coreInitHandler )
CH_DECLARE_MESSAGE_HANDLER( coreTermHandler )
CH_DECLARE_MESSAGE_HANDLER( coreStatusHandler )
CH_DECLARE_MESSAGE_HANDLER( corePaneEventHandler )
CH_DECLARE_MESSAGE_HANDLER( coreGetPageCountHandler )
CH_DECLARE_MESSAGE_HANDLER( coreGetPagesHandler )
CH_DECLARE_MESSAGE_HANDLER( coreGetPageDataHandler )
CH_DECLARE_MESSAGE_HANDLER( coreReleasePagesHandler )

// Menu message handler
CH_DECLARE_MESSAGE_HANDLER( menuMsgDefHandler )

/*----------------------------------------------------------------------------
	ChClientMainInfo class
----------------------------------------------------------------------------*/

class ChClientMainInfo : public ChMainInfo
{
	public:
		ChClientMainInfo( ChModuleID idModule, ChCore* pCore );

	public:
		ChDispatcher	coreDispatcher;
};


/*----------------------------------------------------------------------------
	ChClientCore class
----------------------------------------------------------------------------*/

class ChClientCore : public ChCore
{
	friend class ChMainFrame;
	friend class ChApp;
											/* The following function is
												declared a friend because it
												needs to call the SetValid
												method */

	public:
		enum	tagClientMode { modeNormal = 1, modeDisabled, modeShutdown };

		ChClientCore( ChMainFrame* pFrame );
		virtual ~ChClientCore();

		virtual void NewFrameWnd( const ChString& strArgs,
									const ChString& strLabel = "" ) const;

		virtual bool ActivateFrame( const ChString& strFrameName );

		virtual ChPersistentFrame* GetFrameWnd() const;
		virtual void SetFrameTitle(	const ChString& strDocName );
		virtual ChString GetFrameName() const;
		virtual void SetFrameName( const ChString& strFrameName );

		virtual void EnumerateFrames( ChFrameVisitor& enumFrame );


		virtual void DisplayStatus( const ChString& strStatus );
		virtual void SetStatusPaneText( int iIndex, const ChString& strStatus );

		virtual void UpdateSessionPane( const ChString& strPaneText, bool boolStartTracking = true );


		virtual ChRMenuMgr *GetMenuMgr() { return m_pMenuMgr; }
		virtual ChSplitter* GetSplitter();

		virtual bool IsFlashWindow()		
						{
							if (m_boolFlashWindow) { m_boolWasFlashWindow = true; }
							return m_boolFlashWindow; 
						}
		virtual bool WasFlashWindow()
						{
							bool wasFlash = m_boolWasFlashWindow;
							m_boolWasFlashWindow = false;
							return wasFlash;
						}
		virtual void EnableFlashWindow( bool boolFlash = true )
						{
							m_boolFlashWindow = boolFlash;	
						}
		
		virtual ChString GetModuleName( const ChModuleID& idModule );

		virtual ChMainInfo* GetMainInfo( const ChModuleID& idModule, 
										const char* pstrFrameName = 0 ) const;
		virtual ChMainInfo* GetMainInfo( const ChString& strModule, 
										const char* pstrFrameName = 0 ) const;

		virtual void RegisterDispatcher( const ChModuleID idModule,
									ChDispatcher *pDispatcher );
		virtual void UnregisterDispatcher( const ChModuleID idModule );

		virtual bool AsyncDispatchMsg( const ChModuleID idModule, ChMsg* pMsg );
		virtual chparam DispatchMsg( const ChModuleID idModule, ChMsg& msg );

		virtual void LoadClientModule( const ChString& strModuleName,
								const ChString& strModuleBase,
								const ChModuleID& idNotifyModule = 0,
								chparam userData = 0, bool boolOptional = false,
								bool boolUseExisting = true );

		virtual void UnloadModule( const ChModuleID& idModule ) const;
		virtual void UnloadModule( const ChString& strModule ) const;

		virtual void AbortRequests( bool boolAbortPrefetch = false, 
								ChHTTPStreamManager* pStreamMgr = 0 );
		virtual bool GetURL( const ChString strURL, chuint32 flOptions = 0, 
											ChHTTPStreamManager* pStream = 0,
											chparam userData = 0 );
		virtual bool PostURL( const ChString strURL, const char* pData, chint32 lLen, 
											chuint32 flOptions = 0, 
											ChHTTPStreamManager* pStream = 0,
											chparam userData = 0 );

		virtual void DisplayWebPage( const ChString& strURL, int iBrowser );

		virtual int	 GetModuleCount();
		virtual int GetModuleIDs( int iModuleCount, ChModuleID* pModules );

		ChHTTPConn* GetHTTPConn();

		inline ChArgumentList* GetArgList() { return &m_ArgList; }
		inline ChPuebloModuleManager* GetModuleMgr() { return m_pModuleMgr; }
		inline int GetReqID() { return m_iReqID; }
		inline int GetClientMode() { return m_modeClient; }

		inline bool IsOnline() { return m_boolTrackTime; }

		#if defined( CH_DEBUG )

		inline bool UseLocalModules() { return m_boolUseLocalModules; }

		#endif	// defined( CH_DEBUG )

	protected:

		enum tagStartOptions { 
								doLicense = 0x01, doRegistration = 0x02, 
								doLoginNotify = 0x04, doQuickSplash = 0x80
							 };
							
		void StartPueblo( const ChString& strArg, chuint uOptions = 0 );
		void StartShutdown();

		ChHTTPDDE* GetDDEConn();

		bool DoLicenseDialog();
		bool DoRegistration();
	
		void OnSecondTick( time_t timeCurr );

	private :
		void UpdateSessionTime();

	private:					   
		int							m_modeClient;
		int							m_iReqID;
		chuint32					m_luSecondsInState;
		ChString						m_strSessionText;
		bool						m_boolTrackTime;
		bool						m_boolFlashWindow;
		bool						m_boolWasFlashWindow;
		ChMainFrame*				m_pFrame;
		ChRMenuMgr*					m_pMenuMgr;
		ChPuebloModuleManager*		m_pModuleMgr;
		ChArgumentList				m_ArgList;

		#if defined( CH_DEBUG )

		bool			m_boolUseLocalModules;

		#endif	// defined( CH_DEBUG )


	protected:
};

#endif	// !defined( _CHCLCORE_H )

// Local Variables: ***
// tab-width:4 ***
// End: ***
