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

	This file consists of interfaces used by the Pueblo client core.  This
	file is only used on the client.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#if defined( CH_MSW )

	#include <ChReg.h>

//	#include "ChPrConn.h"
	#include "ChPrDbg.h"
	#include "ChPrFont.h"
	#include "ChPrNet.h"
	#include "ChPrApps.h"
	#include "ChPrefsProxy.h"
	#include "ChPrefsTrace.h"

	#include "ChAbout.h"

#else

	#include <stdio.h>
	#include <ChTypes.h>
	#include <ChModule.h>
	#include <ChModMgr.h>
	#include <ChRMenu.h>
	#include <ChMsgTyp.h>
	#include <ChPerFrm.h>
	#include <ChDialogs.h>

	#define IDCANCEL	1

#endif // defined( CH_MSW )

#include "ChClCore.h"



/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define PREF_PAGE_COUNT				7
#define ABOUT_PAGE_COUNT			4

/*----------------------------------------------------------------------------
	Handler declarations
----------------------------------------------------------------------------*/



static ChMsgHandlerDesc	coreHandlers[] =
					{	
						{CH_MSG_INIT, coreInitHandler},
						{CH_MSG_TERM, coreTermHandler},
						{CH_MSG_STATUS, coreStatusHandler},
						{CH_MSG_GET_PAGE_COUNT, coreGetPageCountHandler},
						{CH_MSG_GET_PAGES, coreGetPagesHandler},
						{CH_MSG_GET_PAGE_DATA, coreGetPageDataHandler},
						{CH_MSG_RELEASE_PAGES, coreReleasePagesHandler},
						};


/*----------------------------------------------------------------------------
	Handler implementations
----------------------------------------------------------------------------*/

CH_IMPLEMENT_MAIN_HANDLER( coreMainHandler )
{
	chparam		retVal = 0;

	switch( msg.GetMessage() )
	{
		case CH_MSG_INIT:
		{
			ChClientMainInfo	*pMainInfo;
			chint16				sHandlerCount = sizeof( coreHandlers ) /
												sizeof( ChMsgHandlerDesc );

											// Construct the main info object

			pMainInfo = new ChClientMainInfo( idModule, pCore );

											// Add specific message handlers

			pMainInfo->coreDispatcher.AddHandler( coreHandlers,
													sHandlerCount );

											/* Return a pointer to the
												ChMainInfo-derived class */
			retVal = (chparam)pMainInfo;
			break;
		}

		case CH_MSG_TERM:
		{
			delete pMainInfo;
			break;
		}
	}

	return retVal;
}

#ifdef CH_MSW

CH_IMPLEMENT_MESSAGE_HANDLER( menuMsgDefHandler )
{
	chparam				retVal = 0;

	// Do nothing

	return retVal;
}


#endif // defined( CH_MSW )


CH_IMPLEMENT_MESSAGE_HANDLER( coreDefHandler )
{
	chparam				retVal = 0;

	TRACE1( "MESSAGE:  UNKNOWN (%ld) (core)\n", msg.GetMessage() );

	return retVal;
}


CH_IMPLEMENT_MESSAGE_HANDLER( coreInitHandler )
{
	return 0;
}


CH_IMPLEMENT_MESSAGE_HANDLER( coreTermHandler )
{
	return 0;
}


CH_IMPLEMENT_MESSAGE_HANDLER( coreStatusHandler )
{
	ChStatusMsg*	pMsg = (ChStatusMsg*)&msg;
	ChString			strStatus;

	pMsg->GetParams( strStatus );
	pMainInfo->GetCore()->DisplayStatus( strStatus );
	return 0;
}


CH_IMPLEMENT_MESSAGE_HANDLER( coreGetPageCountHandler )
{
	ChGetPageCountMsg*	pMsg = (ChGetPageCountMsg*)&msg;
	ChPageType			type;
	int					iPageCount;

	pMsg->GetParams( type );

	switch( type )
	{
		case pagePreferences:
		{
			iPageCount = PREF_PAGE_COUNT;

			#if defined( CH_DEBUG )
			{
				iPageCount++;
			}
			#endif	// defined( CH_DEBUG )
			break;
		}

		case pageAbout:
		{
			iPageCount = ABOUT_PAGE_COUNT;
			break;
		}

		default:
		{
			iPageCount = 0;
			break;
		}
	}

	return iPageCount;
}



CH_IMPLEMENT_MESSAGE_HANDLER( coreGetPagesHandler )
{
	ChGetPagesMsg*		pMsg = (ChGetPagesMsg*)&msg;
	ChPageType			type;
	chint16				sCount;
	chparam*			pPages;

	pMsg->GetParams( type, sCount, pPages );

	switch( type )
	{
		case pagePreferences:
		{
			#if defined( CH_DEBUG )
			{
				ASSERT( PREF_PAGE_COUNT + 1 == sCount );
			}
			#else	// defined( CH_DEBUG )
			{
				ASSERT( PREF_PAGE_COUNT == sCount );
			}
			#endif	// defined( CH_DEBUG )

			#if defined( CH_MSW )
			{
				pPages[0] = (chparam)new ChPrefsCachePage;
				pPages[1] = (chparam)new ChPrefsApps;
				pPages[2] = (chparam)new ChPrefsColorPage;
				pPages[3] = (chparam)new ChPrefsFontPage;
				pPages[4] = (chparam)new ChPrefsNetworkPage;
				pPages[5] = (chparam)new ChPrefsProxyPage;
				pPages[6] = (chparam)new ChPrefsTracePage;

				#if defined( CH_DEBUG )
				{
					pPages[7] = (chparam)new ChPrefsDebugPage;
				}
				#endif	// defined( CH_DEBUG )
			}
			#endif	// defined( CH_MSW )
			break;
		}

		case pageAbout:
		{
			ASSERT( ABOUT_PAGE_COUNT == sCount );

			#if defined( CH_MSW )
			{
				pPages[0] = (chparam)new ChPuebloAbout;
				pPages[1] = (chparam)new ChDisclaimerAbout;
				pPages[2] = (chparam)new ChTeamAbout;
				pPages[3] = (chparam)new ChUEAbout;
			}
			#endif	// defined( CH_MSW )
			break;
		}

		default:
		{
			break;
		}
	}

	return 0;
}


CH_IMPLEMENT_MESSAGE_HANDLER( coreGetPageDataHandler )
{
	ChGetPageDataMsg*	pMsg = (ChGetPageDataMsg*)&msg;
	ChPageType			type;
	chint16				sCount;
	chparam*			pPages;

	pMsg->GetParams( type, sCount, pPages );

	switch( type )
	{
		case pagePreferences:
		{
			#if defined( CH_DEBUG )
			{
				ASSERT( PREF_PAGE_COUNT + 1 == sCount );
			}
			#else	// defined( CH_DEBUG )
			{
				ASSERT( PREF_PAGE_COUNT == sCount );
			}
			#endif	// defined( CH_DEBUG )

			#if defined( CH_MSW )
			{
				ChPropertyPage*	pPage;
				chint16			sLoop;

				for (sLoop = 0; sLoop < sCount; sLoop++)
				{
					if (pPage = (ChPropertyPage*)pPages[sLoop])
					{
						pPage->OnCommit();
					}
				}
			}
			#endif	// defined( CH_MSW )
			break;
		}

		case pageAbout:
		{
			ASSERT( ABOUT_PAGE_COUNT == sCount );

			#if defined( CH_MSW )
			{
				ChPropertyPage*	pPage;
				chint16			sLoop;

				for (sLoop = 0; sLoop < sCount; sLoop++)
				{
					if (pPage = (ChPropertyPage*)pPages[sLoop])
					{
						pPage->OnCommit();
					}
				}
			}
			#endif	// defined( CH_MSW )
			break;
		}

		default:
		{
			break;
		}
	}

	return 0;
}


CH_IMPLEMENT_MESSAGE_HANDLER( coreReleasePagesHandler )
{
	ChReleasePagesMsg*	pMsg = (ChReleasePagesMsg*)&msg;
	ChPageType			type;
	chint16				sCount;
	chparam*			pPages;

	pMsg->GetParams( type, sCount, pPages );

	switch( type )
	{
		case pagePreferences:
		{
			#if defined( CH_DEBUG )
			{
				ASSERT( PREF_PAGE_COUNT + 1 == sCount );
			}
			#else	// defined( CH_DEBUG )
			{
				ASSERT( PREF_PAGE_COUNT == sCount );
			}
			#endif	// defined( CH_DEBUG )

			#if defined( CH_MSW )
			{
				ChPropertyPage*	pPage;
				chint16			sLoop;

				for (sLoop = 0; sLoop < sCount; sLoop++)
				{
					if (pPage = (ChPropertyPage*)pPages[sLoop])
					{
						delete pPage;
					}
				}
			}
			#endif	// defined( CH_MSW )
			break;
		}

		case pageAbout:
		{
			ASSERT( ABOUT_PAGE_COUNT == sCount );
#ifdef CH_MSW
			ChPropertyPage*	pPage;
			chint16			sLoop;

			for (sLoop = 0; sLoop < sCount; sLoop++)
			{
				if (pPage = (ChPropertyPage*)pPages[sLoop])
				{
					delete pPage;
				}
			}
#endif
			break;
		}

		default:
		{
			break;
		}
	}

	return 0;
}

// $Log$
