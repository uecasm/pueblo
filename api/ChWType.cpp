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

	Contains the implementation of the ChWorldType class.

----------------------------------------------------------------------------*/

// $Header$


#include "headers.h"

#include <ChArch.h>

#include "ChWType.h"

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	ChWorldType class
----------------------------------------------------------------------------*/

											/* 'Other' must be the last in
												the following list */
WorldTypeName ChWorldType::m_typeNames[] =
							{	{ Aber, "Aber", unamePwLogin },
								{ Circle, "Circle", unamePwLogin },
								{ DGD, "DGD", unamePwLogin },
								{ Diku, "Diku", unamePwLogin },
								{ LPMud, "LPMud", unamePwLogin },
								{ Merc, "Merc", unamePwLogin },
								{ Moo, "Moo", variableLogin },
								{ Muck, "Muck", connectLogin },
								{ Muse, "Muse", unamePwLogin },
								{ Mux, "Mux", connectLogin },
								{ Talker, "Talker", unamePwLogin },
								{ TinyMush, "TinyMush", connectLogin },
								{ TinyMush, "Mush", connectLogin },		// Backward compatibility
								{ otherType, "Other", variableLogin },
								{ endTypeList, "" } };


ChWorldType::ChWorldType( const ChString& strType )
{
	Set( strType );
}


ChWorldType::ChWorldType( const ChWorldType& type )
{
	m_type = type.m_type;
	m_loginType = type.m_loginType;
}


ChWorldType::ChWorldType( WorldType type )
{
	WorldTypeName*	pTable = ChWorldType::m_typeNames;

	m_type = type;
											// Set the login type
	m_loginType = undefinedLogin;
	while ((endTypeList != pTable->type) && (undefinedLogin == m_loginType))
	{
		if (pTable->type == m_type)
		{
			m_loginType = pTable->login;
		}
		else
		{
			pTable++;
		}
	}
											/* If nothing else was found, use
												'variableLogin' */
	if (undefinedLogin == m_loginType)
	{
		m_loginType = variableLogin;
	}
}


#if defined( CH_MSW )

void ChWorldType::FillTypeList( CComboBox* pCombo )
{
	int		iIndex = 0;
	WorldType	lastType = endTypeList;

	while (m_typeNames[iIndex].type != endTypeList)
	{
		if (m_typeNames[iIndex].type != lastType)
		{
			pCombo->AddString( m_typeNames[iIndex].pstrName );
			lastType = m_typeNames[iIndex].type;
		}

		iIndex++;
	}
}

#endif	// defined( CH_MSW )


void ChWorldType::Serialize( ChArchive& ar )
{
	chuint16	suVersion = constVersion;
	chuint16	suType;
											// First call base class
	ChStreamable::Serialize( ar );
											// Now serialize ourself
	if (modeWrite & ar.GetMode())
	{
		suType = m_type;
											// Write out data
		ar << suType;
	}
	else
	{										// Read in data
		ar >> suType;

		m_type = (WorldType)suType;
	}
}


ChString ChWorldType::GetName() const
{
	WorldTypeName*	pTable = ChWorldType::m_typeNames;
	ChString			strType;

 	while ((endTypeList != pTable->type) && (0 == strType.GetLength()))
 	{
		if (pTable->type == m_type)
		{
			strType = pTable->pstrName;
		}
		else
		{
			pTable++;
		}
	}

	return strType;
}


void ChWorldType::Set( const ChString& strType )
{
	WorldTypeName*	pTable = m_typeNames;
	
	m_type = undefinedType;

	while ((endTypeList != pTable->type) && (undefinedType == m_type))
	{
		if (0 == strType.CompareNoCase( pTable->pstrName ))
		{
			m_type = pTable->type;
			m_loginType = pTable->login;
		}
		else
		{
			pTable++;
		}
	}
											/* If nothing else was found, use
												'Other' */
	if (undefinedType == m_type)
	{
		m_type = otherType;
	}
}


// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
