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

	Definition for the ChWorldMainInfo class.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _WORLD_H )
#define _WORLD_H

#include <ChConn.h>
#include <ChDispat.h>

#include "ChWInfo.h"
#include "ChWorldCmdLine.h"

/*----------------------------------------------------------------------------
	Switches
----------------------------------------------------------------------------*/

#if defined( CH_MSW )

	#define USE_SOUND
											/* Define this to load the
												graphics VRML module on world
												load */
	#define USE_GRAPHICS	1

// UE: This isn't actually used anywhere :)

#endif	// defined( CH_MSW )


/*----------------------------------------------------------------------------
	Includes
----------------------------------------------------------------------------*/


#include "TinTin.h"


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

											// HTML
#define MUD_TEXT_ON			"<xch_mudtext>"
#define MUD_TEXT_OFF		"</xch_mudtext>"

/*----------------------------------------------------------------------------
	Types
----------------------------------------------------------------------------*/

#if !defined( CH_PUEBLO_PLUGIN )
typedef enum { editMenuCut, editMenuCopy, editMenuPaste } EditMenuItem;
#endif

typedef enum { focusNone, focusTextInput, focusTextOutput } FocusTarget;
typedef enum { echoOn, echoTelnetOff, echoAutoOff } EchoState;


/*----------------------------------------------------------------------------
	Forward class declarations
----------------------------------------------------------------------------*/

class ChCore;
class ChWorldMainInfo;
class ChWorldConn;
class ChTextInput;
class ChTextOutput;

#if !defined( CH_PUEBLO_PLUGIN )
class ChMenu;
class ChFileMenu;
class ChEditMenu;
class ChViewMenu;
class ChWindowMenu;
#endif

class ChWorldStreamManager;
class ChConnectingDlg;

/*----------------------------------------------------------------------------
	ChWorldTinTin class
----------------------------------------------------------------------------*/

class ChWorldTinTin : public TinTin
{
	public:
		ChWorldTinTin( ChWorldMainInfo* pMainInfo );
		~ChWorldTinTin() {}

		virtual void SendToWorld( const ChString& strOutput );
		virtual void Display( const ChString& strOutput,
								bool boolPreformatted = false ) const;
};


/*----------------------------------------------------------------------------
	WorldLoadInfo class

		A pointer to an object of this class is passed as user data when
		loading URLs.
----------------------------------------------------------------------------*/

class WorldLoadInfo
{
	public:
		typedef enum { typeModule } LoadType;

	public:
		WorldLoadInfo( const ChString& strModule ) :
				m_strModule( strModule )
					{
					}

		inline LoadType GetType() { return typeModule; }
		inline const ChString& GetModuleName() { return m_strModule; }
	protected:
		ChString		m_strModule;
};


/*----------------------------------------------------------------------------
	ChWorldMainInfo class
----------------------------------------------------------------------------*/

class ChWorldMainInfo : public ChMainInfo
{
	friend class ChWorldConn;
	friend class ChConnectingDlg;

	CH_FRIEND_SOCKET_HANDLER( worldSocketHandler )
	CH_FRIEND_SOCKET_ASYNCHANDLER( worldSocketAsyncHandler )

	CH_FRIEND_MESSAGE_HANDLER( worldGetPageDataHandler )
	CH_FRIEND_MESSAGE_HANDLER( worldLoadCompleteHandler )
	CH_FRIEND_MESSAGE_HANDLER( worldInvalidWorldHandler )

	CH_FRIEND_MESSAGE_HANDLER( OnStdEditCut )
	CH_FRIEND_MESSAGE_HANDLER( OnStdEditCopy )
	CH_FRIEND_MESSAGE_HANDLER( OnStdEditPaste )

	public:
		enum tagTrackUsage { worldConnect = 1, worldDisconnect, worldEnhanced };

		ChWorldMainInfo( const ChModuleID& idModule, ChCore* pCore,
							ChArgumentList* pList );
		virtual ~ChWorldMainInfo();
		
		inline int 	GetConnectID() { return m_iConnectID; }
		inline ChTextInput* GetTextInput() { return m_pTextInput; }
		inline ChTextOutput* GetTextOutput() { return m_pTextOutput; }
		inline ChWorldTinTin* GetTinTin() { return m_pTinTin; }

		inline ChWorldStreamManager* GetStream() { return m_pWorldStreamMgr; }

		inline const ChString& GetCurrentURL() const { return m_strCurrentURL; }
		inline const ChModuleID& GetSoundID() const { return m_idSoundModule; }
		inline const ChModuleID& GetGraphicsID() const { return m_idGraphicsModule; }
		inline ChWorldConn* GetWorldConnection() const { return m_pWorldConn; }
		inline ChWorldInfo* GetWorldInfo() { return m_pWorldInfo; }
		inline ChWorldCmdLine& GetCmdLine()	{ return m_worldCmdLine; }
		inline bool IsFirstPuebloEnhanced() const
						{
							return !m_boolPuebloEnhancedFound;
						}
		inline bool IsLoadPending() const { return m_boolLoadPending; }
		inline void SetLoadPending( bool boolPending )
						{
							m_boolLoadPending = boolPending;
						}
		inline bool IsPersonalList() const { return m_boolPersonalList; }
		inline bool IsShown() const { return m_boolShown; }
		inline bool IsTopLevelWorldList() const
						{
							return (m_urlList.GetCount() <= 1);
						}

		void NotifyCore( ChMsg& msg ) const;
		void NotifySound( ChMsg& msg ) const;
		void NotifyGraphics( ChMsg& msg )  const;

		inline EchoState GetEchoState() const { return m_echoState; }

		inline bool GetNotifyInactive() const { return m_boolNotifyInactive; }
		inline bool GetNotifyFlash() const { return m_boolNotifyFlash; }
		inline bool GetNotifyAlert() const { return m_boolNotifyAlert; }
		inline const ChString& GetNotifyMatch() const { return m_strNotifyMatch; }

#if !defined( CH_PUEBLO_PLUGIN )
		inline ChFileMenu* GetFileMenu()
						{
							return m_boolMenus ? m_pStdFileMenu : 0;
						}
		inline ChEditMenu* GetEditMenu()
						{
							return m_boolMenus ? m_pStdEditMenu : 0;
						}
		inline ChViewMenu* GetViewMenu()
						{
							return m_boolMenus ? m_pStdViewMenu : 0;
						}
		inline ChMenu* GetWorldMenu()
						{
							return m_boolMenus ? m_pWorldMenu : 0;
						}
		inline ChWindowMenu* GetWindowMenu()
						{
							return m_boolMenus ? m_pStdWindowMenu : 0;
						}

#endif //  !defined( CH_PUEBLO_PLUGIN )

		inline bool IsConnected() { return 0 != m_pWorldConn; }

		inline void SetCurrentURL( const ChString& strURL )
						{
							m_strCurrentURL = strURL;
						}
		inline void SetPauseOnDisconnect( bool boolPauseOnDisconnect )
						{
							m_boolPauseOnDisconnect = boolPauseOnDisconnect;
						}

		inline void SetPauseInline( bool boolPauseInline )
						{
							m_boolPauseInline = boolPauseInline;
						}

		virtual void OnSecondTick( time_t timeCurr );

											/* The first two methods here will
												send text to the world, while
												the third will display text
												into the output window */

		void Send( const ChString& strText, bool boolEcho = true );
		void Send( const ChString& strDefaultCmd,
					const ChString& strMD5, const ChString& strOverrideCmd,
					const ChString& strParams, bool boolEcho = true );
		void Display( const ChString& strText, bool boolPreformatted );

		void Initialize();
		void ShowModule( bool boolShow );
		void DisplayWorldList();

		bool DoCommand( const ChString& strCommand, chint32 lX = -1,
							chint32 lY = -1 );
		bool DoInline( const ChString& strArgs );
		bool DoHint( const ChString& strArgs );

		void DoAlert();
#if !defined( CH_PUEBLO_PLUGIN )
		void DoQuickConnect();
#endif

		bool Connect( const ChWorldInfo& info );
		void CreateShortcut( const ChWorldInfo& info );

		void OnInitialStartup();
		void OnPuebloEnhanced( const ChVersion& versEnhanced );
		void DoAutoLogin();
		void ShutdownWorld( bool boolShutdownMessage = true,
							bool boolEntirely = false );
		void CompletePartialShutdown( bool boolEntirely = false );

		void AddCurrentWorld();
		void CreateShortcut();
		void CreateCurrentWorldShortcut();
		bool GetPersonalWorldList();
		void EditPersonalWorldList();

		void DoPreviousURL();

		void SetEchoPrefs( bool boolEcho, bool boolBold, bool boolItalic );

		bool LookForPuebloEnhanced( const ChString& strLine,
									ChVersion& versEnhanced );

		void UpdatePreferences();

		void SetFocusTarget( FocusTarget target, bool boolGainingFocus );

		bool WantTextLines();
		void OnTextLine( const ChString& strLine );
		void LookForNotify( const ChString& strLine ) const;

		void SetEchoState( EchoState newState, bool boolPreserve = false );

		inline const ChString& GetMD5() const { return m_strMD5; }
		inline void SetMD5( const ChString& strMD5 ) { m_strMD5 = strMD5; }
		inline bool VerifyMD5( const ChString& strMD5 ) const
				{
					return strMD5 == m_strMD5;
				}

		void CreateMD5Checksum();

	protected:
		inline FocusTarget GetFocusTarget() { return m_focusTarget; }
		inline const ChString& GetHomePage() { return m_worldCmdLine.GetHomePage(); }

		inline bool DisplayChanged() { return m_boolDisplayChanged; }
		inline void SetDisplayChanged( bool boolChanged = true )
				{
					m_boolDisplayChanged = boolChanged;
				}

		inline void SetNotifyInactive( bool boolNotifyInactive )
						{
							m_boolNotifyInactive = boolNotifyInactive;
						}
		inline void SetNotifyFlash( bool boolNotifyFlash )
						{
							m_boolNotifyFlash = boolNotifyFlash;
						}
		inline void SetNotifyAlert( bool boolNotifyAlert )
						{
							m_boolNotifyAlert = boolNotifyAlert;
						}
		inline void SetNotifyMatch( const ChString& strMatch )
						{
							m_strNotifyMatch = strMatch;
							m_strNotifyMatch.MakeLower();
						}
		inline void SetPersonalList( bool boolPersonal = true )
						{
							m_boolPersonalList = boolPersonal;
						}
		inline void SetShown( bool boolShown = true )
						{
							m_boolShown = boolShown;
						}

		void OnWorldConnect();
		void OnInvalidWorld( const ChString& strReason );

		void OnAsyncSocketAddress( int iError, unsigned long address );
		void OnConnectComplete( int iErrorCode );

		bool DoJump( const ChString& strURL, const ChString& strHTML,
						bool boolForceLoad, bool boolAddToHistory = true,
						bool boolCritical = false );
		void DoXCmd( const ChString& strCommand );
		bool DoXMode( ChString& strArgs );

		void SetSoundID( const ChModuleID& idModule );
		void SetGraphicsID( const ChModuleID& idModule );

#if !defined( CH_PUEBLO_PLUGIN )
		bool CheckEditMenuItem( EditMenuItem item );
		void DoEditMenuItem( EditMenuItem item );
#endif

		void AddChacoListJump();

		void DisplayConnectDlg( const ChWorldInfo& info );
		void CloseConnectDlg();

	private:
		void RegisterDispatchers();

#if !defined( CH_PUEBLO_PLUGIN )
		void CreateMenus();
		void InstallMenus();
		void UninstallMenus();
		void DestroyMenus();
#endif

		void Disconnect();					/* Should only be called from the
												ShutdownWorld method */
		void AddURLToList( const ChString& strURL );
		void EmptyURLList();

		void LoadSoundModule( bool boolOptional = false );
		void UnloadSoundModule();
		void LoadGraphicsModule( bool boolOptional = false );
		void UnloadGraphicsModule();

		void SendConnectedMsg();
		void TrackUsage( int iType );

	private:

		ChTextInput*			m_pTextInput;
		ChTextOutput*			m_pTextOutput;
		ChWorldConn*			m_pWorldConn;
		ChWorldTinTin*			m_pTinTin;

		ChWorldStreamManager*	m_pWorldStreamMgr;

		ChDispatcher			m_worldDispatcher;

		bool					m_boolLoadPending;
		bool					m_boolPuebloEnhancedFound;

		FocusTarget				m_focusTarget;

		ChModuleID				m_idSoundModule;
		ChModuleID		 		m_idGraphicsModule;

		ChWorldInfo*			m_pWorldInfo;

		bool					m_boolShown;
		EchoState				m_echoState;

#if !defined( CH_PUEBLO_PLUGIN )
		bool					m_boolMenus;
		bool					m_boolMenusInstalled;

		ChFileMenu*				m_pStdFileMenu;
		ChEditMenu*				m_pStdEditMenu;
		ChViewMenu*				m_pStdViewMenu;
		ChMenu*					m_pWorldMenu;
		ChWindowMenu*			m_pStdWindowMenu;
#endif

		bool					m_boolPersonalList;
		bool					m_boolDisplayChanged;
		bool					m_boolPauseOnDisconnect;
		bool					m_boolPauseInline;
		bool					m_boolPartialShutdown;

		ChParamList				m_urlList;

		ChString					m_strCurrentURL;

		bool					m_boolNotifyInactive;
		bool					m_boolNotifyFlash;
		bool					m_boolNotifyAlert;
		ChString					m_strNotifyMatch;
		ChWorldCmdLine			m_worldCmdLine;
		int						m_iConnectID;

		ChConnectingDlg*		m_pConnectingDlg;
		bool					m_boolWaitingForHostName;

		CRITICAL_SECTION		m_critsecDisconnect;

		ChString					m_strMD5;
		ChString					m_strPuebloClientParams;
};


#endif	// !defined( _WORLD_H )

// $Log$
// Revision 1.1.1.1  2003/02/03 18:53:43  uecasm
// Import of source tree as at version 2.53 release.
//
