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

	This file contains the implementation of the names list, responsible
	for managing a list of names stored for the user.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <ChReg.h>

#include "ChNList.h"


/*----------------------------------------------------------------------------
	ChNameList class
----------------------------------------------------------------------------*/

ChNameList::ChNameList( const ChString& strGroup, const ChString& strLabel,
						char cDelim ) :
				reg( CH_COMPANY_NAME, CH_PRODUCT_NAME, strGroup ),
				m_strLabel( strLabel ), m_cDelim( cDelim ),
				m_boolData( false ), m_boolDirty( false ), m_sCount( -1 )
{
}


ChNameList::~ChNameList()
{
	if (m_boolDirty)
	{
		reg.Write( m_strLabel, m_strList );
	}
}


void ChNameList::Add( const ChString& strName )
{
	if (0 == GetCount())
	{										// First name
		m_strList = strName;

		m_boolDirty = true;					// Write this out during destruct
		m_sCount = 1;
	}
	else
	{										// Add a new name
		if (!IsInList( strName ))
		{
			m_strList += m_cDelim;
			m_strList += strName;

			m_boolDirty = true;				// Write this out during destruct
			m_sCount++;						// Add one to the count
		}
	}

}


chint16 ChNameList::GetCount()
{
	if (!m_boolData)
	{
		Load();
	}

	if (m_sCount < 0)
	{										// The count is not yet cached
		const char	*pstrBuffer = m_strList;

		m_sCount = 0;

		while (*pstrBuffer != 0)
		{
			if (*pstrBuffer != m_cDelim)
			{								// Found a name
				m_sCount++;

				do
				{							/* Skip the weird case of extra
												delimiters */
					pstrBuffer++;
				}
				while ((*pstrBuffer != m_cDelim) && (*pstrBuffer != 0));
			}

			while (*pstrBuffer == m_cDelim)
			{								/* Skip delimiters */
				pstrBuffer++;
			}
		}
	}

	return m_sCount;
}


bool ChNameList::GetName( chint16& sStartIndex, ChString& strName )
{
	bool		boolNameFound = false;

	if (!m_boolData)
	{
		Load();
	}

	if (sStartIndex < m_strList.GetLength())
	{
		const char	*pstrBuffer = m_strList;

		pstrBuffer = &(pstrBuffer[sStartIndex]);

		while (*pstrBuffer == m_cDelim)
		{									/* Skip the weird case of extra
												delimiters */
			pstrBuffer++;
			sStartIndex++;
		}

		if (sStartIndex < m_strList.GetLength())
		{
											/* There are still characters, so
												copy them into the destination
												string */
			strName = "";
			boolNameFound = true;

			while ((*pstrBuffer != m_cDelim) && (*pstrBuffer != 0))
			{
				strName += *pstrBuffer;
				pstrBuffer++;
				sStartIndex++;
			}
		}
	}

	return boolNameFound;
}


bool ChNameList::IsInList( const ChString& strName )
{
	bool	boolInList = false;

	if (GetCount())
	{										// Search the list item by item
		chint16		sIndex = 0;
		ChString		strOneName;

		while (!boolInList && GetName( sIndex, strOneName ))
		{
			if (0 == strName.CompareNoCase( strOneName ))
			{
											// Found it!
				boolInList = true;
			}
		}
	}

	return boolInList;
}


void ChNameList::Load()
{
	reg.Read( m_strLabel, m_strList );
	m_sCount = -1;
	m_boolData = true;
}

// $Log$
