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

	TinTin alias-related methods.  Originally modified from TinTin++,
	(T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t, originally coded by
	Peter Unold 1992.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"
#if !defined(CH_PUEBLO_PLUGIN)
#include "resource.h"
#else
#include "vwrres.h"
#endif

#include "World.h"

#include "TinTin.h"
#include "MemDebug.h"


/*----------------------------------------------------------------------------
	TinTin class
----------------------------------------------------------------------------*/

void TinTin::OnSecondTick( time_t timeCurr )
{
	if (IsTicking())
	{
		ChString		strFormat;
		ChString		strMessage;
		int			iTickSize = GetTickSize();
		time_t		timeStart = GetTimeStart();
		int			iSecondsToTick;
											/* Ignore the time that is passed
												to us */
		timeCurr = time(0);

		iSecondsToTick = iTickSize - ((timeCurr - timeStart) % iTickSize);
		SetSecondsToTick( iSecondsToTick );

		if ((iSecondsToTick == iTickSize) || (iSecondsToTick == 10))
		{
			if (iSecondsToTick == iTickSize)
			{
				LOADSTRING( IDS_TINTIN_TICK_EVENT, strFormat );
				strMessage.Format( strFormat, (const char*)GetTickKey() );
				ActMessage( strMessage );
			}
			else
			{
				LOADSTRING( IDS_TINTIN_TICK_EVENT_10, strFormat );
				strMessage.Format( strFormat, (const char*)GetTickKey() );
				ActMessage( strMessage );
			}
		}
	}
}


/*----------------------------------------------------------------------------
	TinTin protected methods
----------------------------------------------------------------------------*/

void TinTin::DoTick()
{
	ChString		strFormat;
	ChString		strMessage;

	if (IsTicking())
	{
		LOADSTRING( IDS_TINTIN_TICK_LEFT_FMT, strFormat );
		strMessage.Format( strFormat, GetSecondsToTick() );
		Message( strMessage );
	}
	else
	{
		LOADSTRING( IDS_TINTIN_TICK_OFF, strMessage );
		Message( strMessage );
	}
}


void TinTin::DoTickOff()
{
	ChString		strMessage;

	SetTicking( false );

	LOADSTRING( IDS_TINTIN_TICK_SET_OFF, strMessage );
	Message( strMessage );
}


void TinTin::DoTickOn()
{
	ChString		strFormat;
	ChString		strMessage;

	DoTickSet();
	SetTicking( true );

	LOADSTRING( IDS_TINTIN_TICK_SET_ON, strFormat );
	strMessage.Format( strFormat, GetTickSize() );
	Message( strMessage );
}


void TinTin::DoTickSet()
{											// Reset the ticker
	time_t		timeCurr = time(0);

	SetTimeStart( timeCurr );
	SetSecondsToTick( GetTickSize() );
}


void TinTin::DoTickSize( const ChString& strArgs )
{
	ChString		strSize;
	ChString		strFormat;
	ChString		strMessage;

	GetArgInBraces( strArgs, strSize, true );

	if (!strSize.IsEmpty())
	{
		const char*		pstrSize = strSize;
		bool			boolSuccess = false;

		if (isdigit( *pstrSize ))
		{
			int		iSize = atoi( pstrSize );

			if ((iSize > 0) && (iSize <= 32000))
			{
				SetTickSize( iSize );
				DoTickSet();

				LOADSTRING( IDS_TINTIN_TICK_SIZE_SET, strMessage );
				Message( strMessage );

				boolSuccess = true;
			}
		}

		if (!boolSuccess)
		{
			LOADSTRING( IDS_TINTIN_TICK_SIZE_SET_ERR, strFormat );
			strMessage.Format( strFormat, (int)32000 );
			Message( strMessage );
		}
	}
	else
	{
		LOADSTRING( IDS_TINTIN_TICK_SIZE_SET_USAGE, strMessage );
		Message( strMessage );
	}
}     


void TinTin::DoTickKey( const ChString& strArgs )
{
	ChString		strKey;
	ChString		strFormat;
	ChString		strMessage;

	GetArgInBraces( strArgs, strKey, true );

	if (!strKey.IsEmpty())
	{
		SetTickKey( strKey );

		LOADSTRING( IDS_TINTIN_TICK_KEY_SET, strFormat );
		strMessage.Format( strFormat, (const char*)GetTickKey() );
		Message( strMessage );
	}
	else
	{
		LOADSTRING( IDS_TINTIN_TICK_KEY, strFormat );
		strMessage.Format( strFormat, (const char*)GetTickKey() );
		Message( strMessage );
	}
}

// $Log$
// Revision 1.1.1.1  2003/02/03 18:53:42  uecasm
// Import of source tree as at version 2.53 release.
//
