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

	Contains the implementation of the ChWorldList class.

----------------------------------------------------------------------------*/

// $Header$


#include "headers.h"
#include "ChWInfo.h"
#include "ChWList.h"


/*----------------------------------------------------------------------------
	ChWorldList class
----------------------------------------------------------------------------*/

ChWorldList::ChWorldList() :
				m_boolDirty( false ),
				m_pReg( 0 )
{
	chint16		sTemp;

	m_pReg = new ChRegistry( WORLD_LIST_GROUP );
	ASSERT( 0 != m_pReg );

	if (m_pReg->Read( WORLD_LIST_COUNT, sTemp ))
	{
											// Read the list (new format)
		Load();
	}
	else
	{										// Read the old list
		delete m_pReg;
		m_pReg = new ChRegistry( WORLD_LIST_GROUP_OLD );
		ASSERT( 0 != m_pReg );

		Load();
											/* Now save the old list to the
												new location (using the new
												format) */
		delete m_pReg;
		m_pReg = new ChRegistry( WORLD_LIST_GROUP );
		ASSERT( 0 != m_pReg );

		Store();
	}
}


ChWorldList::~ChWorldList()
{
	if (m_pReg)
	{
		delete m_pReg;
		m_pReg = 0;
	}
}


void ChWorldList::Load()
{
	ChString		strOneWorld;
	chint16		sKeyCount;
											// Empty the list
	m_list.Empty();
	
	if (m_pReg->Read( WORLD_LIST_COUNT, sKeyCount ))
	{
		chint16		sLoop;
											/* Worlds are written sequentially,
											   one string per world
											   description, with a zero byte
											   between worlds.  A double zero
											   byte follows the last
											   description. */
		
		for (sLoop = 1; sLoop <= sKeyCount; sLoop++)
		{
			char			label[30];
			ChString			strOneWorld;
			ChWorldInfo*	pInfo;
			
			sprintf( label, WORLD_LIST_ITEM, (int)sLoop );
			if (m_pReg->Read( label, strOneWorld ))
			{
				pInfo = new ChWorldInfo( strOneWorld );
				
				if (pInfo->IsValid() && (pInfo->GetName().GetLength() > 0))
				{
					m_list.AddTail( pInfo );
				}
				else
				{
					delete pInfo;
				}
			}
		}
	}
}


void ChWorldList::Store()
{
	ChString		strOneWorld;
	ChPosition	pos;
	chint16		sCounter = 0;
											/* Worlds are written sequentially,
											   one string per world
											   description, with a zero byte
											   between worlds.  A double zero
											   byte follows the last
											   description. */
	pos = m_list.GetHeadPosition();
	while (pos)
	{
		char		label[30];
		ChWorldInfo*	pInfo = m_list.Get( pos );
		
		pInfo->Stringize( strOneWorld );
											// Format the label
		sCounter++;
		sprintf( label, WORLD_LIST_ITEM, (int)sCounter );
			
											// Write a single world
		m_pReg->Write( label, strOneWorld );
		
		m_list.GetNext( pos );
	}

	m_pReg->Write( WORLD_LIST_COUNT, sCounter );
}


void ChWorldList::Add( const ChString& strName, const ChString& strDesc,
						const ChString& strHost, chint16 sPort,
						const ChWorldType& type, ChLoginType login,
						const ChString& strUsername, const ChString& strPassword,
						const ChString& strHomePage )
{
	ChPosition	pos = FindNamePos( strName, strUsername );

	if (pos)
	{										/* Already in the list --
												change exising item */
		ChWorldInfo*	pInfo = m_list.Get( pos );

		pInfo->Set( strDesc, strHost, sPort, type, login, strUsername,
					strPassword, strHomePage );
	}
	else
	{										// Add new item

		ChWorldInfo*	pItem = new ChWorldInfo( strName, strDesc, strHost,
													sPort, type, login,
													strUsername, strPassword,
													strHomePage );
		m_list.AddTail( pItem );
	}
}


void ChWorldList::Add( const ChWorldInfo& info )
{
	ChString		strName = info.GetName();

	if (strName.GetLength() == 0)
	{
		strName = info.GetHost();

		ASSERT( strName.GetLength() > 0 );
	}

	Add( strName, info.GetDesc(), info.GetHost(), info.GetPort(),
			info.GetType(), info.GetLoginType(),
			info.GetUsername(), info.GetPassword(), info.GetHomePage() );
}


void ChWorldList::Remove( const ChString& strName, const ChString& strUsername )
{
	ChPosition		pos;

	pos = FindNamePos( strName, strUsername );
	if (pos)
	{
		m_list.Remove( pos );
	}
}


ChWorldInfo* ChWorldList::FindName( const ChString& strName,
									const ChString& strUsername )
{
	ChPosition		pos = FindNamePos( strName, strUsername );
	ChWorldInfo*	pInfo;

	if (pos)
	{
		pInfo = m_list.Get( pos );
	}
	else
	{
		pInfo = 0;
	}

	return pInfo;
}


/*----------------------------------------------------------------------------
	ChWorldList class protected methods
----------------------------------------------------------------------------*/

ChPosition ChWorldList::FindNamePos(  const ChString& strName,
										const ChString& strUsername )
{
	ChPosition		pos;
	bool			boolFound = false;

	pos = m_list.GetHeadPosition();
	while (pos && !boolFound)
	{
		ChWorldInfo*	pInfo = m_list.Get( pos );

		if ((pInfo->GetName() == strName) &&
			(pInfo->GetUsername() == strUsername))
		{
			boolFound = true;
		}
		else
		{
			m_list.GetNext( pos );
		}
	}

	return pos;
}


// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
