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

	Implementation of stream handler for ChSound.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"


#include <ChHTTP.h>
//#include <ChUtil.h>	

#include <ChCore.h>
#include <ChMsgTyp.h>
#include "ChSoundInfo.h"

#include "ChSoundStream.h"
#include "MemDebug.h"


ChSoundStreamManager::ChSoundStreamManager( ChSoundMainInfo* pMainInfo ) :
						m_pMainInfo( pMainInfo )
{
}


int ChSoundStreamManager::NewStream( chparam requestData,
										pChHTTPStream pStream,
										bool boolSeekable )
{
	return streamAsFile;
}


void ChSoundStreamManager::StreamAsFile( chparam requestData,
											pChHTTPStream pStream,
											const char* pstrFilename )
{
}

void ChSoundStreamManager::DestroyStream( chparam requestData,
											pChHTTPStream pStream,
											int iReason )
{
	if (iReason)
	{										/* Error - do we need to notify
														the user? */

		ChSoundInfo*	pSoundInfo = (ChSoundInfo*)requestData;

		delete pSoundInfo;
	}
	else
	{
		ChLoadCompleteMsg *pMsg = new ChLoadCompleteMsg( pStream->GetURL(),
										pStream->GetCacheFilename(),
										pStream->GetMimeType(), requestData );
		ASSERT( pMsg );

		m_pMainInfo->GetCore()->AsyncDispatchMsg( m_pMainInfo->GetModuleID(),
													pMsg );
	}
}

// $Log$
// Revision 1.1.1.1  2003/02/03 18:53:02  uecasm
// Import of source tree as at version 2.53 release.
//
