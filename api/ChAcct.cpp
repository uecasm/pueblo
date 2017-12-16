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

	This file consists of the implementation of the ChAccountInfo class.

----------------------------------------------------------------------------*/

#include "headers.h"

#include <ChArch.h>
#include <ChAcct.h>

#include <MemDebug.h>

/*----------------------------------------------------------------------------
	ChAccountInfo constructor and destructor
----------------------------------------------------------------------------*/

ChAccountInfo::ChAccountInfo()
{
	m_gender = noyb;
}

ChAccountInfo::~ChAccountInfo()
{
}


/*----------------------------------------------------------------------------
	ChAccountInfo public methods
----------------------------------------------------------------------------*/

void ChAccountInfo::SetName( const ChString& strFirstName,
								const ChString& strMiddleName,
								const ChString& strSurname )
{
	SetFirstName( strFirstName );
	SetMiddleName( strMiddleName );
	SetSurname( strSurname );
}

void ChAccountInfo::SetAddress( const ChString& strStreet,
								const ChString& strApt, const ChString& strCity,
								const ChString& strState, const ChString& strZip,
								const ChString& strCountry,
								const ChString& strEMail )
{
	SetStreet( strStreet );
	SetApartment( strApt );
	SetCity( strCity );
	SetState( strState );
	SetZip( strZip );
	SetCountry( strCountry );
	SetEMail( strEMail );
}


void ChAccountInfo::AddMiscField( const ChString& strLabel,
								const ChString& strValue )
{
	m_strMisc += strLabel + "=\"" + strValue + "\"\t";
}


void ChAccountInfo::Serialize( ChArchive& archive )
{
	ChStreamable::Serialize( archive );

						// Now serialize ourself

	if (modeWrite & archive.GetMode())
	{
						// Write out data

		archive << m_strFirstName << m_strMiddleName << m_strSurname
				<< m_strStreet << m_strApt
				<< m_strCity << m_strState << m_strZip
				<< m_strCountry
				<< m_strEMail
				<< (chint16)m_gender
				<< m_strMisc;
	}
	else
	{					// Read in data

		chint16		sGender;

		archive >> m_strFirstName >> m_strMiddleName >> m_strSurname
				>> m_strStreet >> m_strApt
				>> m_strCity >> m_strState >> m_strZip
				>> m_strCountry
				>> m_strEMail
				>> sGender
				>> m_strMisc;

		m_gender = (Gender)sGender;
	}
}
