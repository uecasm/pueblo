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

	This file consists of implementation of the ChList class.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"
#include <ChList.h>

#include <MemDebug.h>


ChParamList::ChParamList() : m_pHead( 0 ), m_pTail( 0 ), m_lCount( 0 )
{
}

ChParamList::~ChParamList()
{
	Empty();
	ASSERT( m_lCount == 0 );
}


chint32 ChParamList::GetCount() const
{
	return m_lCount;
}

bool ChParamList::IsEmpty() const
{
	return m_lCount == 0;
}


chparam& ChParamList::GetHead()
{
	ASSERT( m_pHead != 0 );
	return m_pHead->item;
}

chparam ChParamList::GetHead() const
{
	ASSERT( m_pHead != 0 );
	return m_pHead->item;
}

chparam& ChParamList::GetTail()
{
	ASSERT( m_pTail != 0 );
	return m_pTail->item;
}

chparam ChParamList::GetTail() const
{
	ASSERT( m_pTail != 0 );
	return m_pTail->item;
}


chparam& ChParamList::GetNext( ChPosition& pos )
{
	ChParamNode*	pNode = (ChParamNode*)pos;

	#if defined( CH_MSW )
	{
		ASSERT( AfxIsValidAddress( pNode, sizeof( ChParamNode ) ) );
	}
	#endif	// defined( CH_MSW )

	pos = (ChPosition)pNode->pNext;
	return pNode->item;
}


chparam ChParamList::GetNext( ChPosition& pos ) const
{
	ChParamNode*	pNode = (ChParamNode*)pos;

	#if defined( CH_MSW )
	{
		ASSERT( AfxIsValidAddress( pNode, sizeof( ChParamNode ) ) );
	}
	#endif	// defined( CH_MSW )

	pos = (ChPosition)pNode->pNext;
	return pNode->item;
}


chparam& ChParamList::GetPrev( ChPosition& pos )
{
	ChParamNode*	pNode = (ChParamNode*)pos;

	#if defined( CH_MSW )
	{
		ASSERT( AfxIsValidAddress( pNode, sizeof( ChParamNode ) ) );
	}
	#endif	// defined( CH_MSW )

	pos = (ChPosition)pNode->pPrev;
	return pNode->item;
}


chparam ChParamList::GetPrev( ChPosition& pos ) const
{
	ChParamNode*	pNode = (ChParamNode*)pos;

	#if defined( CH_MSW )
	{
		ASSERT( AfxIsValidAddress( pNode, sizeof( ChParamNode ) ) );
	}
	#endif	// defined( CH_MSW )

	pos = (ChPosition)pNode->pPrev;
	return pNode->item;
}


chparam& ChParamList::Get( ChPosition pos )
{
	ChParamNode*	pNode = (ChParamNode*)pos;

	#if defined( CH_MSW )
	{
		ASSERT( AfxIsValidAddress( pNode, sizeof( ChParamNode ) ) );
	}
	#endif	// defined( CH_MSW )

	return pNode->item;
}


chparam ChParamList::Get( ChPosition pos ) const
{
	ChParamNode* pNode = (ChParamNode*)pos;

	#if defined( CH_MSW )
	{
		ASSERT( AfxIsValidAddress( pNode, sizeof( ChParamNode ) ) );
	}
	#endif	// defined( CH_MSW )

	return pNode->item;
}


void ChParamList::Set( ChPosition pos, chparam newVal )
{
	ChParamNode*	pNode = (ChParamNode*)pos;

	#if defined( CH_MSW )
	{
		ASSERT( AfxIsValidAddress( pNode, sizeof( ChParamNode ) ) );
	}
	#endif	// defined( CH_MSW )

	pNode->item = newVal;
}


void ChParamList::Empty()
{
	ChParamNode*	pOldHead;
											// Destroy elements
	while (pOldHead = m_pHead)
	{
		chparam		item = pOldHead->item;
											// Call the destructor helper
		ChDestructHelper( &item );

		m_pHead = pOldHead->pNext;
		FreeNode( pOldHead );
	}

	m_lCount = 0;
	m_pHead = m_pTail = 0;
}

ChPosition ChParamList::GetHeadPosition() const
{
	return (ChPosition)m_pHead;
}

ChPosition ChParamList::GetTailPosition() const
{
	return (ChPosition)m_pTail;
}

ChPosition ChParamList::AddHead( chparam newVal )
{
	ChParamNode*	pNewNode;

	pNewNode = NewNode( 0, m_pHead );
	pNewNode->item = newVal;
	if (0 != m_pHead)
	{
		m_pHead->pPrev = pNewNode;
	}
	else
	{
		m_pTail = pNewNode;
	}
	m_pHead = pNewNode;

	return (ChPosition)pNewNode;
}


ChPosition ChParamList::AddTail( chparam newVal )
{
	ChParamNode*	pNewNode;

	pNewNode = NewNode( m_pTail, 0 );

	pNewNode->item = newVal;
	if (m_pTail != 0)
	{
		m_pTail->pNext = pNewNode;
	}
	else
	{
		m_pHead = pNewNode;
	}
	m_pTail = pNewNode;

	return (ChPosition)pNewNode;
}


void ChParamList::AddHead( ChParamList* pNewList )
{
	ChPosition	pos;

	ASSERT( pNewList != 0 );
											/* add a list of same elements to
												head (maintain order) */
	pos = pNewList->GetTailPosition();
	while (pos != 0)
	{
		AddHead( pNewList->GetPrev( pos ) );
	}
}


void ChParamList::AddTail( ChParamList* pNewList )
{
	ChPosition	pos;

	ASSERT( pNewList != 0 );
											// add a list of same elements
	pos = pNewList->GetHeadPosition();
	while (pos != 0)
	{
		AddTail( pNewList->GetNext( pos ) );
	}
}


chparam ChParamList::RemoveHead()
{
	ASSERT( m_pHead != 0 );				// don't call on empty list

	ChParamNode*	pOldNode = m_pHead;
	chparam			returnValue = pOldNode->item;

	m_pHead = pOldNode->pNext;
	if (m_pHead != 0)
	{
		m_pHead->pPrev = 0;
	}
	else
	{
		m_pTail = 0;
	}
	FreeNode( pOldNode );

	return returnValue;
}


chparam ChParamList::RemoveTail()
{
	ASSERT( m_pTail != 0 );				// don't call on empty list

	ChParamNode*	pOldNode = m_pTail;
	chparam			returnValue = pOldNode->item;

	m_pTail = pOldNode->pPrev;
	if (m_pTail != 0)
	{
		m_pTail->pNext = 0;
	}
	else
	{
		m_pHead = 0;
	}
	FreeNode( pOldNode );

	return returnValue;
}


ChPosition ChParamList::InsertBefore( ChPosition pos, chparam newVal )
{
	if (pos == 0)
	{
		return AddHead( newVal );
	}

	ChParamNode*	pOldNode = (ChParamNode*)pos;
	ChParamNode*	pNewNode = NewNode( pOldNode->pPrev, pOldNode );
	
	pNewNode->item = newVal;

	if (pOldNode->pPrev != 0)
	{
		#if defined( CH_MSW )
		{
			ASSERT( AfxIsValidAddress( pOldNode->pPrev, sizeof( ChParamNode ) ) );
		}
		#endif	// defined( CH_MSW )

		pOldNode->pPrev->pNext = pNewNode;
	}
	else
	{
		ASSERT( pOldNode == m_pHead );

		m_pHead = pNewNode;
	}
	pOldNode->pPrev = pNewNode;

	return (ChPosition)pNewNode;
}


ChPosition ChParamList::InsertAfter( ChPosition pos, chparam newVal )
{
	ChParamNode*	pOldNode;
	ChParamNode*	pNewNode;

	if (pos == 0)
	{
		return AddTail( newVal );
	}

	pOldNode = (ChParamNode*)pos;

	#if defined( CH_MSW )
	{
		ASSERT( AfxIsValidAddress( pOldNode, sizeof( ChParamNode ) ) );
	}
	#endif	// defined( CH_MSW )

	pNewNode = NewNode( pOldNode, pOldNode->pNext );
	pNewNode->item = newVal;

	if (pOldNode->pNext != 0)
	{
		#if defined( CH_MSW )
		{
			ASSERT( AfxIsValidAddress( pOldNode->pNext,
										sizeof( ChParamNode ) ) );
		}
		#endif	// defined( CH_MSW )

		pOldNode->pNext->pPrev = pNewNode;
	}
	else
	{
		ASSERT( pOldNode == m_pTail );

		m_pTail = pNewNode;
	}
	pOldNode->pNext = pNewNode;

	return (ChPosition)pNewNode;
}


void ChParamList::Remove( ChPosition pos )
{
	ChParamNode*	pOldNode;

	pOldNode = (ChParamNode*)pos;

	#if defined( CH_MSW )
	{
		ASSERT( AfxIsValidAddress( pOldNode, sizeof( ChParamNode ) ) );
	}
	#endif	// defined( CH_MSW )

											// remove pOldNode from list
	if (pOldNode == m_pHead)
	{
		m_pHead = pOldNode->pNext;
	}
	else
	{
		#if defined( CH_MSW )
		{
			ASSERT( AfxIsValidAddress( pOldNode->pPrev,
										sizeof( ChParamNode ) ) );
		}
		#endif	// defined( CH_MSW )

		pOldNode->pPrev->pNext = pOldNode->pNext;
	}

	if (pOldNode == m_pTail)
	{
		m_pTail = pOldNode->pPrev;
	}
	else
	{
		#if defined( CH_MSW )
		{
			ASSERT( AfxIsValidAddress( pOldNode->pNext,
										sizeof( ChParamNode ) ) );
		}
		#endif	// defined( CH_MSW )

		pOldNode->pNext->pPrev = pOldNode->pPrev;
	}

	FreeNode( pOldNode );
}


ChPosition ChParamList::FindIndex( chint32 lIndex ) const
{
	ChParamNode*	pNode;

	ASSERT( lIndex >= 0 );

	if (lIndex >= m_lCount)
	{
		return 0;
	}

	pNode = m_pHead;
	while (lIndex--)
	{
		#if defined( CH_MSW )
		{
			ASSERT( AfxIsValidAddress( pNode, sizeof( ChParamNode ) ) );
		}
		#endif	// defined( CH_MSW )

		pNode = pNode->pNext;
	}

	return (ChPosition)pNode;
}


ChPosition ChParamList::Find( chparam searchValue,
								ChPosition startAfter ) const
{
	ChParamNode*	pNode;

	pNode = (ChParamNode*)startAfter;
	if (pNode == 0)
	{
		pNode = m_pHead;
	}
	else
	{
		#if defined( CH_MSW )
		{
			ASSERT( AfxIsValidAddress( pNode, sizeof( ChParamNode ) ) );
		}
		#endif	// defined( CH_MSW )

		pNode = pNode->pNext;				// start after the one specified
	}

	for (; pNode != 0; pNode = pNode->pNext)
	{
		if (ChCompareHelper( pNode->item, searchValue ))
		{
			return (ChPosition)pNode;
		}
	}

	return 0;
}


ChParamNode* ChParamList::NewNode( ChParamNode* pPrev, ChParamNode* pNext )
{
	ChParamNode*	pNode;
											// Create the new node
	pNode = new ChParamNode;
											// Stick the node into the chain
	pNode->pPrev = pPrev;
	pNode->pNext = pNext;
											// Increment the node count
	m_lCount++;
	ASSERT( m_lCount > 0 );
											// Call construct helper
	ChConstructHelper( &pNode->item );

	return pNode;
}


void ChParamList::FreeNode( ChParamNode* pNode )
{
	delete pNode;
											// Decrement the node count
	m_lCount--;
	ASSERT( m_lCount >= 0 );
}


// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
