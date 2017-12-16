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

	This file contains the implementation of the ChDispatcher class.

----------------------------------------------------------------------------*/

#include "headers.h"
#include <ChDispat.h>
#include <ChModule.h>
#include <ChCore.h>
#include <ChSplay.h>

#include <MemDebug.h>

ChDispatcher::ChDispatcher( ChCore* pCore, const ChModuleID idModule,
							const ChMsgHandler defHandler ) :
		m_pCore( pCore ),
		m_defHandler( defHandler ),
		m_idModule( idModule ),
		m_pMainInfo( 0 )
{
	ASSERT( m_pCore );	
											// Register this new dispatcher

	m_pCore->RegisterDispatcher( m_idModule, this );
}

ChDispatcher::~ChDispatcher()
{											// Unregister the dispatcher

	m_pCore->UnregisterDispatcher( m_idModule );
}

void ChDispatcher::AddHandler( chint32 lMessage, const ChMsgHandler handler )
{
	ChMsgHandler*	pHandler;
											/* First check to see if a
												record exists */

	if (pHandler = (ChMsgHandler*)m_handlerTree.Find( lMessage ))
	{
											/* The message was found.  We'll
												update the handler */
		*pHandler = handler;
	}
	else
	{										/* The message was not found, so
												add it */

		m_handlerTree.Insert( lMessage, (chparam)handler );
	}
}

void ChDispatcher::AddHandler( const ChMsgHandlerDesc* pHandlerDescs,
								chint16 sCount )
{
	chint16					sLoop;
	const ChMsgHandlerDesc	*pCurr = pHandlerDescs;

	for (sLoop = 0; sLoop < sCount; sLoop++)
	{
		AddHandler( pCurr->lMessage, pCurr->handler );
		pCurr++;
	}
}


chparam ChDispatcher::Dispatch( ChMsg &msg )
{
	ChMsgHandler*	pHandler;
	chparam			retVal;
	chint32			lMessage = msg.GetMessage();

	pHandler = (ChMsgHandler*)m_handlerTree.Find( lMessage );

											/* If we don't have the pMainInfo
												information, look for it */
	if (0 == m_pMainInfo)
	{
		m_pMainInfo = m_pCore->GetMainInfo( m_idModule );

		ASSERT( m_pMainInfo );				// Should not be zero at this point
	}
											// Call the handler
	if (pHandler)
	{
		retVal = (*pHandler)( msg, m_pMainInfo );
	}
	else
	{
		retVal = m_defHandler( msg, m_pMainInfo );
	}
											/* Don't do anything after calling
												the handler, as CH_MSG_TERM
												probably causes the handler to
												be deleted, and it will no
												longer be valid */

	return( retVal );
}


// Local Variables: ***
// tab-width:4 ***
// End: ***
