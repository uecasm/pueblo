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

	This file consists of the implementation of the ChKeyMap class, use to
	map keys to functionality.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <ChKeyMap.h>

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	ChKeyMapItem class
----------------------------------------------------------------------------*/

ChKeyMapItem::ChKeyMapItem() :
			m_luKey( 0 ),
			m_flModifiers( 0 ),
			m_userData( 0 )
{
}


ChKeyMapItem::ChKeyMapItem( chuint32 luKey, chflag32 flModifiers,
							const ChString& strName, chparam userData ) :
			m_luKey( luKey ),
			m_flModifiers( flModifiers ),
			m_strName( strName ),
			m_userData( userData )
{
}


ChKeyMapItem::~ChKeyMapItem()
{
}


/*----------------------------------------------------------------------------
	ChKeyMapItem public methods
----------------------------------------------------------------------------*/

void ChKeyMapItem::Serialize( ChArchive& archive )
{
	ChStreamable::Serialize( archive );
											// Now serialize ourself
	if (modeWrite & archive.GetMode())
	{
		archive << m_luKey << m_flModifiers << m_strName << m_userData;
	}
	else
	{
		archive >> m_luKey >> m_flModifiers >> m_strName >> m_userData;
	}
}


/*----------------------------------------------------------------------------
	ChKeyMap class
----------------------------------------------------------------------------*/

ChKeyMap::ChKeyMap()
{
}


ChKeyMap::~ChKeyMap()
{
	ChPosition	pos;

	while (pos = m_itemList.GetHeadPosition())
	{
		DeleteItem( pos );
	}
}


void ChKeyMap::AddItem( chuint32 luKey, chflag32 flModifiers,
						const ChString& strName, chparam userData )
{
	ChPosition		pos;

	if (pos = FindItem( strName ))
	{
		ChKeyMapItem*	pItem = GetItem( pos );

		pItem->SetKey( luKey );
		pItem->SetModifiers( flModifiers );
		pItem->SetUserData( userData );
	}
	else
	{
		ChKeyMapItem*	pItem = new ChKeyMapItem( luKey, flModifiers, strName,
													userData );

		m_itemList.AddTail( (chparam)pItem );
	}
}


void ChKeyMap::AddItem( const ChKeyMapItem& item )
{
	ChPosition		pos;

	if (pos = FindItem( item.GetName() ))
	{
		ChKeyMapItem*	pItem = GetItem( pos );

		pItem->SetKey( item.GetKey() );
		pItem->SetModifiers( item.GetModifiers() );
		pItem->SetUserData( item.GetUserData() );
	}
	else
	{
		ChKeyMapItem*	pItem = new ChKeyMapItem( item );

		m_itemList.AddTail( (chparam)pItem );
	}
}


ChPosition ChKeyMap::FindItem( const ChString& strName ) const
{
	ChPosition		pos = m_itemList.GetHeadPosition();

	while (pos)
	{
		ChKeyMapItem*	pItem = GetItem( pos );

		ASSERT( pItem );

		if (pItem->GetName() == strName)
		{
			break;
		}
		else
		{
			m_itemList.GetNext( pos );
		}
	}

	return pos;
}


ChPosition ChKeyMap::FindItem( chuint32 luKey, chflag32 flModifiers ) const
{
	ChPosition		pos = m_itemList.GetHeadPosition();

	while (pos)
	{
		ChKeyMapItem*	pItem = GetItem( pos );

		ASSERT( pItem );

		if ((pItem->GetKey() == luKey) &&
			(pItem->GetModifiers() == flModifiers))
		{
			break;
		}
		else
		{
			m_itemList.GetNext( pos );
		}
	}

	return pos;
}


ChKeyMapItem* ChKeyMap::GetItem( ChPosition pos ) const
{
	ASSERT( 0 != pos );

	return (ChKeyMapItem*)m_itemList.Get( pos );
}


void ChKeyMap::DeleteItem( ChPosition pos )
{
	ASSERT( 0 != pos );

	delete (ChKeyMapItem*)m_itemList.Get( pos );
	m_itemList.Remove( pos );
}


void ChKeyMap::Empty()
{
	ChPosition	pos;

	while (pos = m_itemList.GetHeadPosition())
	{
		DeleteItem( pos );
	}
}


/*----------------------------------------------------------------------------
	ChKeyMap public methods
----------------------------------------------------------------------------*/

void ChKeyMap::Serialize( ChArchive& archive )
{
	ChStreamable::Serialize( archive );
											// Now serialize ourself
	if (modeWrite & archive.GetMode())
	{
	}
	else
	{
	}
}

// $Log$
