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

	This file contains the interface for the ChVersion class.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <stdlib.h>
#include <stdio.h>

#include <ChArch.h>
#include <ChVers.h>

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	ChVersion public methods
----------------------------------------------------------------------------*/

void ChVersion::operator =( double version )
{
	char	buffer[32];
	char	*pstrBuffer = buffer;
											/* Take the integer part of the
												number as the major version
												number */
	m_sMajor = (chint16)version;
											/* Now take the decimal part as
												the minor version number */
	version -= (long)version;
	sprintf( buffer, "%#lf", version );
	while ('.' != *pstrBuffer)
	{
		pstrBuffer++;
	}
	pstrBuffer++;							// Move past the decimal

	m_sMinor = (chint16)atoi( pstrBuffer );
}

void ChVersion::operator =( chuint32 luPackedVersion )
{
	m_sMajor = HIWORD( luPackedVersion );
	m_sMinor = LOWORD( luPackedVersion );

	m_strLabel = "";
}


bool ChVersion::operator <( const ChVersion& versCompare ) const
{
	bool	boolLessThan;

	if (m_sMajor < versCompare.GetMajor())
	{
		boolLessThan = true;
	}
	else if (m_sMajor == versCompare.GetMajor())
	{
		if (m_sMinor < versCompare.GetMinor())
		{
			boolLessThan = true;
		}
		else
		{
			boolLessThan = false;
		}
	}
	else
	{
		boolLessThan = false;
	}

	return boolLessThan;
}


void ChVersion::Serialize( ChArchive& ar )
{
											/* First call the parents'
												Serialize method */
	ChStreamable::Serialize( ar );
											// Now serialize ourself
	if (ar.GetMode() & modeRead)
	{										// read

		ar >> m_sMajor >> m_sMinor >> m_strLabel;
	}
	else
	{										// write

		ar << m_sMajor << m_sMinor << m_strLabel;
	}
}

ChString ChVersion::Format( int iOption, int iDecimalPlaces ) const
{
	ChString	strFormat;
	ChString	strVersion;

	if (0 == iDecimalPlaces)
	{
		strFormat = "%hd.%hd";
	}
	else
	{
		strFormat.Format( "%%hd.%%0%dhd", iDecimalPlaces );
	}

	strVersion.Format( strFormat, m_sMajor, m_sMinor );

	if ((iOption != formatShort) && (0 != m_strLabel.GetLength()))
	{
		strVersion += " (" + m_strLabel + ")";
	}

	return strVersion;
}

// $Log$

// Local Variables: ***
// tab-width:4 ***
// End: ***
