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

	This file contains the implementation of the ChConn class, used to
	manage a connection.

		sockbuf			(sockinetbuf on the client)
			ChConn

----------------------------------------------------------------------------*/

#include <string.h>

#include "headers.h"

#include <ChConn.h>
#include <ChMsg.h>

#include <MemDebug.h>

/*----------------------------------------------------------------------------
	ChConn class
----------------------------------------------------------------------------*/

void ChConn::SendBlock( const void *pBuf, chint32 lLen ) const
{
	int error_count = 0;

	// Yeah, the 1000 is kinda bogus, but we don't want to get stuck
	// here forever.  I'm not sure whether anyone else will close the
	// socket if the SendBlock fails (because SendBlock doesn't return
	// anything).
	while ((lLen != 0) && (error_count < 1000))
	{
		int wcnt = write( pBuf, (int)lLen );

		if (wcnt == -1)
		{
			error ( "user::SendBlock" );
			error_count++;
		}
		else
		{
			lLen -= wcnt;
		}
	}
}


void ChConn::SendLine( const char *pstrLine ) const
{
	ChString	strLine( pstrLine );

	strLine += "\r\n";
	SendBlock( strLine, strLine.GetLength() );
}


// Local Variables: ***
// tab-width:4 ***
// End: ***
