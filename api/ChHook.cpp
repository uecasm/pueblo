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

	This file consists of the interface for the ChHookManager class, which
	provides functional support for a module to manager hooked messages.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"
#include <ChCore.h>

#include <ChHook.h>

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	ChHookManager public methods
----------------------------------------------------------------------------*/

void ChHookManager::Install( const ChModuleID& idModule )
{
											/* Only add each module ID to the
												hook list once */
	if (0 == m_moduleList.Find( idModule ))
	{										/* This module ID isn't in the hook
												list, so we'll add it */
		m_moduleList.AddTail( idModule );
	}
}

chparam ChHookManager::Dispatch( ChMsg& msg, bool& boolProcessed ) const
{
	chparam		returnVal;

	if (m_moduleList.IsEmpty())
	{
		boolProcessed = false;

		returnVal = 0;
	}
	else
	{
		ChPosition		posCurr = m_moduleList.GetHeadPosition();

		do {
			ChModuleID	idModule = m_moduleList.GetNext( posCurr );

			msg.SetDestinationModule( idModule );
			returnVal = m_pCore->DispatchMsg( idModule, msg );

		} while (posCurr && !msg.IsProcessed());

		if (!(boolProcessed = msg.IsProcessed()))
		{
			returnVal = 0;
		}
	}

	return returnVal;
}

void ChHookManager::Promote( const ChModuleID& idModule, bool boolPromote )
{
	ChPosition		pos;

	ASSERT( !m_moduleList.IsEmpty() );
											// Find the element to be promoted
	pos = m_moduleList.Find( idModule );
											// The element must be in the list
	ASSERT( 0 != pos );
											/* Remove the module ID and
												reinsert it at the head or
												tail of the list */
	m_moduleList.Remove( pos );

	if (boolPromote)
	{
		m_moduleList.AddHead( idModule );
	}
	else
	{
		m_moduleList.AddTail( idModule );
	}
}

void ChHookManager::Uninstall( const ChModuleID& idModule )
{
	ChPosition		pos;

	ASSERT( !m_moduleList.IsEmpty() );
											// Find the element to be promoted
	pos = m_moduleList.Find( idModule );
											// The element must be in the list
	ASSERT( 0 != pos );
											// Remove the module ID
	m_moduleList.Remove( pos );
}

// $Log$
