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

	This file consists of the interface for the ChData class.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"
#include <ChData.h>

#if defined( CH_UNIX )
	#define ASSERT_VALID( object )
#endif

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	ChData Constructor and Destructor
----------------------------------------------------------------------------*/

ChData::ChData( ChDataMode mode ) : m_mode( mode )
{
}

ChData::~ChData()
{
}

chflag16 ChData::GetMode()
{
	return m_mode;
}


/*----------------------------------------------------------------------------
	ChData public methods
----------------------------------------------------------------------------*/

void ChData::Flush()
{
	ASSERT_VALID( this );
}

void ChData::Close()
{
}

void ChData::Abort()
{
	ASSERT_VALID( this );

	Close();
}

/*----------------------------------------------------------------------------
	ChData public methods (should not be used by module authors)
----------------------------------------------------------------------------*/

chuint32 ChData::GetPosition() const
{
	return 0;
}

void ChData::SetLength( chuint32 luNewLen )
{
}

chuint32 ChData::Read( void* pBuffer, chuint32 luCount )
{
	ASSERT_VALID( this );

	return 0;
}

void ChData::Write( const void *pBuffer, chuint32 luCount )
{
	ASSERT_VALID( this );
}

chint32 ChData::Seek( chint32 lOff, chuint16 suFrom )
{
	ASSERT_VALID( this );

	return 0;
}

chuint32 ChData::GetBufferPtr( chuint16 suCommand, chuint32, void**, void** )
{
											/* ChData doesn't support "direct
												buffering" interaction with
												ChArchive */
	ASSERT( suCommand == bufferCheck );
	suCommand;								// Not used in retail build

	return 0;   							// Not supported
}

// $Log$
