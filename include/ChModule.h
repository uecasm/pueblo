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

	This file consists of the interface for the following classes:
	
		ChModule
		ChMainInfo

	This file also contains module interface definitions and should be
	included by every module.

----------------------------------------------------------------------------*/

// $Header$

#if (!defined( _CHMODULE_H ))
#define _CHMODULE_H

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif

#include <iostream>
#include <ChTypes.h>
#include <ChStrmbl.h>
#include <ChVers.h>
#include <ChMsg.h>
#include <ChHook.h>


/*----------------------------------------------------------------------------
	Forward class declarations
----------------------------------------------------------------------------*/

class ChPaneMgr;
class ChCore;
class ChArgumentList;

/*----------------------------------------------------------------------------
	Module constants
----------------------------------------------------------------------------*/

#define CH_CORE_MODULE_ID		(2)			// For both server and client

#ifdef __BORLANDC__
 #define CH_MAIN_NAME			"ChMainEntry"
#else
 #define CH_MAIN_NAME			"_ChMainEntry@24"
#endif


/*============================================================================
				ChMainInfo class
============================================================================*/

class CH_EXPORT_CLASS ChMainInfo
{
	friend class ChDispatcher;

	public:
		virtual ~ChMainInfo() {}

		inline ChModuleID GetModuleID() const { return m_idModule; }

											/* The following method is called
												at least once a second */

		virtual void OnSecondTick( time_t timeCurr ) {}

		ChString GetModuleName() const;
		ChCore* GetCore() const { return m_pCore; }
		ChPaneMgr* GetPaneMgr() const;

		#if defined( CH_CLIENT )

		void LoadClientModule( const ChString& strModuleName,
								const ChString& strModuleBase,
								const ChModuleID idNotifyModule = 0,
								chparam userData = 0,
								bool boolOptional = false,
								bool boolUseExisting = true );

		void UnloadClientModule( const ChModuleID& idModule ) const;

		#endif	// defined( CH_CLIENT )


	protected:
											// Hook management methods

		inline void InstallHook( chint32 lMessage, const ChModuleID& idModule )
					{
						GetHookMgr( lMessage )->Install( idModule );
					}

		inline void UninstallHook( chint32 lMessage,
									const ChModuleID& idModule )
					{
						GetHookMgr( lMessage )->Uninstall( idModule );
					}

		inline void PromoteHook( chint32 lMessage, const ChModuleID& idModule,
									bool boolPromote )
					{
						GetHookMgr( lMessage )->Promote( idModule,
															boolPromote );
					}

		inline bool DoHook( ChMsg& msg )
					{
						bool			boolProcessed;
						ChHookManager*	pHookMgr;

						pHookMgr = GetHookMgr( msg.GetMessage() );
						pHookMgr->Dispatch( msg, boolProcessed );

						return boolProcessed;
					}

	protected:
		ChHookManager* GetHookMgr( chint32 lMessage );

	protected:
		ChMainInfo( ChModuleID idModule, ChCore* pCore );

	private :
		ChModuleID				m_idModule;
		ChCore*					m_pCore;
};


/*----------------------------------------------------------------------------
	Module definition macros:
----------------------------------------------------------------------------*/

											/* The following macro declares a
												module library entry point */
#define ChMain	CH_GLOBAL_LIBRARY( chparam ) \
				ChMainEntry( ChMsg &msg, ChCore *pCore, ChMainInfo *pMainInfo, \
								ChModuleID idModule, const ChString *pstrModule, \
								ChArgumentList *pArgList )

											/* The following two macros are
												used to declare and implement
												message handlers */
#define CH_DECLARE_MESSAGE_HANDLER( name ) \
				CH_EXTERN_CALLBACK( chparam ) \
				name( ChMsg &msg, ChMainInfo *pMainInfo );
								
#define CH_FRIEND_MESSAGE_HANDLER( name ) \
				friend CH_GLOBAL_CALLBACK( chparam ) \
				name( ChMsg &msg, ChMainInfo *pMainInfo );
								
#define CH_IMPLEMENT_MESSAGE_HANDLER( name ) \
				CH_GLOBAL_CALLBACK( chparam ) \
				name( ChMsg &msg, ChMainInfo *pMainInfo )

											/* The following macros and
												typedefs are for Chaco
												internal use only */
#define CH_DECLARE_MAIN_HANDLER( name ) \
				CH_EXTERN_CALLBACK( chparam ) \
				name( ChMsg &msg, ChCore *pCore, ChMainInfo *pMainInfo, \
						ChModuleID idModule, const ChString *pstrModule, \
						ChArgumentList *pArgList );

#define CH_IMPLEMENT_MAIN_HANDLER( name ) \
				CH_GLOBAL_CALLBACK( chparam ) \
				name( ChMsg &msg, ChCore *pCore, ChMainInfo *pMainInfo, \
						ChModuleID idModule, const ChString *pstrModule, \
						ChArgumentList *pArgList )

CH_TYPEDEF_LIBRARY( chparam, ChMainHandler )( ChMsg& msg, \
												ChCore *pCore, \
												ChMainInfo *pMainInfo, \
												ChModuleID idModule, \
												const ChString *pstrModule,
												ChArgumentList *pArgList );

CH_TYPEDEF_CALLBACK( chparam, ChMsgHandler )( ChMsg& msg,
												ChMainInfo *pMainInfo );

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR    
#endif

#endif	// !defined( _CHMODULE_H )

// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
