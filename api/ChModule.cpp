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

	This file consists of implementation of the ChModule class.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"
#include <ChCore.h>
#include <ChArch.h>
#include <ChVers.h>
#include <ChModule.h>
#include <ChHook.h>

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	ChMainInfo static variables
----------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------
	ChMainInfo class
----------------------------------------------------------------------------*/

ChMainInfo::ChMainInfo( ChModuleID idModule, ChCore* pCore ) : 
					m_idModule( idModule ),
					m_pCore( pCore )

{
	ASSERT( m_pCore );
}


ChString ChMainInfo::GetModuleName() const
{
	return m_pCore->GetModuleName( m_idModule );
}


ChPaneMgr* ChMainInfo::GetPaneMgr() const
{
	return m_pCore->GetPaneMgr();
}

ChHookManager* ChMainInfo::GetHookMgr( chint32 lMessage )
{
	ChHookManager*		pMgr;

	switch( lMessage )
	{
		case CH_MSG_CMD:
		{
			pMgr = m_pCore->m_phookCommand;
			break;
		}

		case CH_MSG_INLINE:
		{
			pMgr = m_pCore->m_phookInline;
			break;
		}

		default:
		{
			ASSERT( false );
			pMgr = 0;
			break;
		}
	}

	return pMgr;
}



#if defined( CH_CLIENT )

void ChMainInfo::LoadClientModule( const ChString& strModuleName,
									const ChString& strModuleBase,
									const ChModuleID idNotifyModule,
									chparam userData, bool boolOptional,
									bool boolUseExisting )
{

	m_pCore->LoadClientModule( strModuleName, strModuleBase, idNotifyModule,
								userData, boolOptional, boolUseExisting );
}


void ChMainInfo::UnloadClientModule( const ChModuleID& idModule ) const
{

	m_pCore->UnloadModule( idModule );
}

#endif	// defined( CH_CLIENT )


// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
