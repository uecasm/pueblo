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

	This file consists of the interface for the ChMsg class.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHMSG_H )
#define _CHMSG_H
#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif

#include <ChMData.h>
//#include <ChMsgCon.h>
#include <ChVers.h>


/*----------------------------------------------------------------------------
	Chaco message definitions
----------------------------------------------------------------------------*/

#define CH_MSG_GREETING				2   // S->C;  Greeting / Connect successful
#define CH_MSG_LOGIN				3   // C->S;  Login request
#define CH_MSG_INIT					4   //     ;  INIT handler
#define CH_MSG_TERM					5	//     ;  TERM handler
#define CH_MSG_NEW_ACCOUNT			6	// C->S;  New account request
#define CH_MSG_VALIDATION			7	// S->C;  result of login
#define CH_MSG_LOAD_MODULE			8	// C->S;  Request to load a client mod
										// S->C;  Command to load a client mod
#define CH_MSG_ACCOUNT_INFO			9	// C->S;  Sends account info

										// 10 left unused

#define CH_MSG_LOAD_COMPLETE		11	// Requested URL has been down loaded
#define CH_MSG_LOAD_ERROR			12	// Error loading URL
#define CH_MSG_LOAD_DATA			13	// Data for the requested URL

#define CH_MSG_LOGOUT				14	// <-->;  Requests and confirms logout
#define CH_MSG_SHOW_MODULE			15	/* C->CM; Informs client module to
													show or hide itself */
#define CH_MSG_CHECKPT_MODULE		16	// -->CM; Informs client module to
										//			checkpoint itself
#define CH_MSG_MENU_SELECT			17	// Client menu message
#define CH_MSG_MENU_SHOW			18	// Client menu message
#define CH_MSG_MENU_CHANGED			19	// Client menu message

										/* The following hook messages are
											either client<->client or
											server<->server. */
#define CH_MSG_INSTALL_HOOK			20	// Requests a message hook
#define CH_MSG_PROMOTE_HOOK			21	// Promotes or demotes a hook
#define CH_MSG_UNINSTALL_HOOK		22	// Uninstalls one or all hooks

#define CH_MSG_RESET				23	/* C->CM; Informs client module to
													reset its state */

#define CH_MSG_LOAD_FILE			24	// ->CM; Loads a file

										// 25 - 29 left unused

#define CH_MSG_LOAD_IMAGE			30	// S->C;  show this
#define CH_MSG_LOAD_CAST			31	// S->C;  load this castmember
#define CH_MSG_LOAD_ANCHOR			32	// S->C;  define this event trigger
#define CH_MSG_LOAD_SCENE			33	// S->C;  load scene; new or addition
#define CH_MSG_PLAY_GRAPHIC			34	// S->C;  play anim or castmember T/F
#define CH_MSG_SHOW_CAST			35	// S->C;  Show castmember, T/F
#define CH_MSG_ENABLE_DRAG			36	// S->C;  Enable castmember drag T/F
#define CH_MSG_ANCHOR_EVENT			37	// C->S;  Had a hit on a trigger

										// 10 left unused

#define CH_MSG_CMD					40	/* C->C;  Message sent when hotspot
													is clicked */
#define CH_MSG_INLINE				41	/* C->C;  Message sent when an inline
													image is first displayed */
#define CH_MSG_HINT					42	/* C->C;  Message sent when the mouse
													is over a hotspot */
//#define CH_MSG_PREFETCH				43	/* C->C;  Message sent when the HTML parser detects
//													a <xch_prefetch> tag */

										// 44 - 59 left unused

#define CH_MSG_STATUS				60	// -->C;  Status line text to client
										//			core
#define CH_MSG_SET_FOCUS			70	// ->CM;  Command to take the focus

#define CH_MSG_MEDIA_PLAY			80	// ->CM;  Command to play (and
										//			optionally load) media
#define CH_MSG_MEDIA_STOP			81	// ->CM;  Command to stop playing media

#define CH_MSG_GET_PAGE_COUNT		90	// ->CM;  Get number of pages for type
#define CH_MSG_GET_PAGES			91	// ->CM;  Get pages for type
#define CH_MSG_GET_PAGE_DATA		92	// ->CM;  Get data from pages
#define CH_MSG_RELEASE_PAGES		93	// ->CM;  Release pages for type

										// 94 - 99 left unused

#define CH_MSG_USAGE_STATISTIC		100		// C->S;  Usage info to server
#define CH_MSG_INVALID_WORLD		101		// S->CM; World is invald - disconnect
#define CH_MSG_SPROING				102		// S->C, C->S; Like ICMP "ping"
#define CH_MSG_REDIRECT				103		// C->S;  Switch to diff msg server

#define CH_MSG_AD					200		// S->CM; Display an ad

#define CH_MSG_SEND_WORLD_CMD		300		// ->CM; Send a cmd to the world
#define CH_MSG_CONNECTED			310		// ->CM; We're connected to a world

#define CH_MSG_USER					10000	// First user message value


/*----------------------------------------------------------------------------
	ChMsgOrigin class
----------------------------------------------------------------------------*/

//typedef ChMsgConn	ChMsgOrigin;


/*----------------------------------------------------------------------------
	ChMsg class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChMsg : public ChMemData
{
//	friend class ChMsgConn;
//	friend class ChClientModule;
//	friend class ChServerModule;

	public:
		ChMsg( chint32 lMessage, chparam param1 = 0, chparam param2 = 0 );
		ChMsg( chint32 lMessage, const ChVersion& version,
				chparam param1 = 0, chparam param2 = 0 );
		virtual ~ChMsg() {}

		inline ChModuleID GetDestinationModule() const { return m_idDestModule; }
		inline ChModuleID GetOriginModule() const { return m_idOriginModule; }
		inline chint32 GetMessage() const { return m_lMessage; }
		inline chparam GetParam1() const { return m_param1; }
		inline chparam GetParam2() const { return m_param2; }
		inline const ChVersion& GetVersion() const { return m_version; }
		inline bool IsProcessed() const { return m_boolProcessed; }
//		inline const ChMsgOrigin* GetOrigin() const
//					{ return (ChMsgOrigin *)m_pOrigin; }

		inline chint32 SetMessage( chint32 lMessage )
					{
						chint32 lOldMessage = m_lMessage;
						m_lMessage = lMessage;
						return lOldMessage;
					}
		inline chparam SetParam1( chparam param1 )
					{
						chparam oldParam = m_param1;
						m_param1 = param1;
						return oldParam;
					}
		inline chparam SetParam2( chparam param2 )
					{
						chparam oldParam = m_param2;
						m_param2 = param2;
						return oldParam;
					}
		inline void SetVersion( const ChVersion& version )
					{
						m_version = version;
					}
		inline bool SetProcessed( bool boolProcessed = true )
					{
						bool	boolOldProcessed = m_boolProcessed;
						m_boolProcessed = boolProcessed;
						return boolOldProcessed;
					}
		inline void SetDestinationModule( ChModuleID idDest )
					{ m_idDestModule = idDest; }
		inline void SetOriginModule( ChModuleID idOrigin )
					{ m_idOriginModule = idOrigin; }

											// The following should be protected
	protected:
//		inline void SetOrigin( ChMsgConn *pOrigin )
//					{ m_pOrigin = pOrigin; }

	protected:
		chint32 	m_lMessage;
		chparam		m_param1;
		chparam		m_param2;
		ChVersion	m_version;
		bool		m_boolProcessed;

		ChModuleID	m_idOriginModule;
		ChModuleID	m_idDestModule;
		
//		ChMsgConn	*m_pOrigin;
};
 
#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR    
#endif

#endif	// !defined( _CHMSG_H )

// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
