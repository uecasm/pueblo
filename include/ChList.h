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

	This file consists of the ChList class.

----------------------------------------------------------------------------*/

// $Header$

#if (!defined( _CHLIST_H ))
#define _CHLIST_H

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif

#include <ChTypes.h> 


#if defined( CH_MSW ) && defined( WIN32 )
											/* Disable warnings about non-
												exported classes for MSW */
	#pragma warning( disable: 4275 )
	#pragma warning( disable: 4251 )

#endif	// defined( CH_MSW )


#if defined( CH_UNIX )

	#define CH_UNIX_INLINE inline

#else	// defined( CH_UNIX )

	#define CH_UNIX_INLINE

#endif	// defined( CH_UNIX )


/*----------------------------------------------------------------------------
	ChPosition type - Used as a position within a list.  It's really just
						an opaque type for a pointer.
----------------------------------------------------------------------------*/

typedef void*	ChPosition;


#if !defined( NO_TEMPLATES )

/*----------------------------------------------------------------------------
	Template helper functions -- These may be overridden
----------------------------------------------------------------------------*/

template<class TYPE>
inline void ChConstructHelper( TYPE* pItem )
{											/* The default constructor helper
												does nothing with the item */
}

template<class TYPE>
inline void ChDestructHelper( TYPE* pItem )
{											/* The default destructor helper
												does nothing with the item */
}


template<class TYPE>
inline bool ChCompareHelper( const TYPE& item1, const TYPE& item2 )
{
	return item1 == item2;
	//return 1;
}


/*----------------------------------------------------------------------------
	ChListNode template class
----------------------------------------------------------------------------*/

template<class TYPE> class ChListNode
{
	public:
		ChListNode<TYPE>*	pNext;
		ChListNode<TYPE>*	pPrev;
		TYPE				item;
};

#else

#include <TemplCls\ChLstHlp.inl>

#endif // !defined ( NO_TEMPLATES )


/*----------------------------------------------------------------------------
	ChParamNode class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChParamNode
{
	public:
		ChParamNode() {}
		virtual ~ChParamNode() {}

	public:
		ChParamNode*	pNext;
		ChParamNode*	pPrev;
		chparam			item;
};


/*----------------------------------------------------------------------------
	ChParamList class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChParamList
{
	public:
		ChParamList();
		virtual ~ChParamList();

		chint32 GetCount() const;
		bool IsEmpty() const;

		chparam& GetHead();
		chparam GetHead() const;
		chparam& GetTail();
		chparam GetTail() const;
											/* Get head or tail (and remove
												it) - don't call on empty
												list */
		chparam RemoveHead();
		chparam RemoveTail();
											/* Add before head or after tail -
												returns position of new
												element */
		ChPosition AddHead( chparam newVal );
		ChPosition AddTail( chparam newVal );

											/* Add another list of elements
												before head or after tail */
		void AddHead( ChParamList* pSrcList );
		void AddTail( ChParamList* pSrcList );
											/* Delete all elements from the
												list */
		void Empty();

		ChPosition GetHeadPosition() const;
		ChPosition GetTailPosition() const;
		chparam& GetNext( ChPosition& pos );
		chparam GetNext( ChPosition& pos ) const;
		chparam& GetPrev( ChPosition& pos );
		chparam GetPrev( ChPosition& pos ) const;

											/* Get or change an element
												at a given position */
		chparam& Get( ChPosition pos );
		chparam Get( ChPosition pos ) const;
		void Set( ChPosition pos, chparam newVal );
		void Remove( ChPosition pos );
											/* Inserting before or after a
												given position */

		ChPosition InsertBefore( ChPosition pos, chparam newVal );
		ChPosition InsertAfter( ChPosition pos, chparam newVal );

		ChPosition Find( chparam searchValue, ChPosition startAfter = 0 ) const;
		ChPosition FindIndex( chint32 lIndex ) const;

	protected:
		ChParamNode* NewNode( ChParamNode* pPrev, ChParamNode* pNext );
		void FreeNode( ChParamNode* pNode );

	protected:
		ChParamNode*	m_pHead;
		ChParamNode*	m_pTail;
		chint32			m_lCount;
};


#if !defined( NO_TEMPLATES )

/*----------------------------------------------------------------------------
	ChList template class
----------------------------------------------------------------------------*/

template<class TYPE> class ChPtrList : public ChParamList
{
	public:
		ChPtrList() {}
		virtual ~ChPtrList();

		TYPE* GetHead();
		const TYPE* GetHead() const;
		TYPE* GetTail();
		const TYPE* GetTail() const;
											/* Get head or tail (and remove
												it) - don't call on empty
												list */
		TYPE* RemoveHead();
		TYPE* RemoveTail();
											/* Add before head or after tail -
												returns position of new
												element */
		ChPosition AddHead( const TYPE* pNewVal );
		ChPosition AddTail( const TYPE* pNewVal );

											/* Add another list of elements
												before head or after tail */

		void AddHead( ChPtrList<TYPE>* pSrcList );
		void AddTail( ChPtrList<TYPE>* pSrcList );

											/* Delete all elements from the
												list */
		void Empty();

		TYPE* GetNext( ChPosition& pos );
		const TYPE* GetNext( ChPosition& pos ) const;
		TYPE* GetPrev( ChPosition& pos );
		const TYPE* GetPrev( ChPosition& pos ) const;

											/* Get or change an element
												at a given position */
		TYPE* Get( ChPosition pos );
		const TYPE* Get( ChPosition pos ) const;
		void Set( ChPosition pos, const TYPE* pNewVal );
		void Remove( ChPosition pos );

											/* Inserting before or after a
												given position */

		ChPosition InsertBefore( ChPosition pos, const TYPE* pNewVal );
		ChPosition InsertAfter( ChPosition pos, const TYPE* pNewVal );

		ChPosition Find( const TYPE* pSearchValue,
							ChPosition startAfter = 0 ) const;
};


/*----------------------------------------------------------------------------
	ChPtrList public inline methods
----------------------------------------------------------------------------*/

template<class TYPE>
CH_UNIX_INLINE ChPtrList<TYPE>::~ChPtrList()
{
	Empty();
	ASSERT( m_lCount == 0 );
}

template<class TYPE>
inline TYPE* ChPtrList<TYPE>::GetHead()
{
	return (TYPE*)ChParamList::GetHead();
}

template<class TYPE>
inline const TYPE* ChPtrList<TYPE>::GetHead() const
{
	return (const TYPE*)ChParamList::GetHead();
}

template<class TYPE>
inline TYPE* ChPtrList<TYPE>::GetTail()
{
	return (TYPE*)ChParamList::GetTail();
}

template<class TYPE>
inline const TYPE* ChPtrList<TYPE>::GetTail() const
{
	return (const TYPE*)ChParamList::GetTail();
}

template<class TYPE>
inline TYPE* ChPtrList<TYPE>::GetNext( ChPosition& pos )
{
	return (TYPE*)ChParamList::GetNext( pos );
}

template<class TYPE>
inline const TYPE* ChPtrList<TYPE>::GetNext( ChPosition& pos ) const
{
	return (const TYPE*)ChParamList::GetNext( pos );
}

template<class TYPE>
inline TYPE* ChPtrList<TYPE>::GetPrev( ChPosition& pos )
{
	return (TYPE*)ChParamList::GetPrev( pos );
}

template<class TYPE>
inline const TYPE* ChPtrList<TYPE>::GetPrev( ChPosition& pos ) const
{
	return (const TYPE*)ChParamList::GetPrev( pos );
}

template<class TYPE>
inline TYPE* ChPtrList<TYPE>::Get( ChPosition pos )
{
	return (TYPE*)ChParamList::Get( pos );
}

template<class TYPE>
inline const TYPE* ChPtrList<TYPE>::Get( ChPosition pos ) const
{
	return (const TYPE*)ChParamList::Get( pos );
}

template<class TYPE>
inline void ChPtrList<TYPE>::Set( ChPosition pos, const TYPE* pNewVal )
{
	ChParamList::Set( pos, (chparam)pNewVal );
}

template<class TYPE>
CH_UNIX_INLINE void ChPtrList<TYPE>::Remove( ChPosition pos )
{
	ChParamNode*	pNode;
	TYPE*			pItem;

	pNode = (ChParamNode*)pos;
	pItem = (TYPE*)pNode->item;
											// Call the destructor helper
	ChDestructHelper( pItem );
											// Call the parent class
	ChParamList::Remove( pos );
}

template<class TYPE>
CH_UNIX_INLINE void ChPtrList<TYPE>::Empty()
{
	ChParamNode*	pOldHead;
											// Destroy elements
	while (pOldHead = m_pHead)
	{
		TYPE*	pItem = (TYPE*)pOldHead->item;

											// Call the destructor helper
		ChDestructHelper( pItem );

		m_pHead = pOldHead->pNext;
		FreeNode( pOldHead );
	}

	m_lCount = 0;
	m_pHead = m_pTail = 0;
}

template<class TYPE>
inline ChPosition ChPtrList<TYPE>::AddHead( const TYPE* pNewVal )
{
	return ChParamList::AddHead( (chparam)pNewVal );
}

template<class TYPE>
inline ChPosition ChPtrList<TYPE>::AddTail( const TYPE* pNewVal )
{
	return ChParamList::AddTail( (chparam)pNewVal );
}

template<class TYPE>
CH_UNIX_INLINE void ChPtrList<TYPE>::AddHead( ChPtrList* pNewList )
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

template<class TYPE>
CH_UNIX_INLINE void ChPtrList<TYPE>::AddTail( ChPtrList* pNewList )
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

template<class TYPE>
inline TYPE* ChPtrList<TYPE>::RemoveHead()
{
	return (TYPE*)ChParamList::RemoveHead();
}

template<class TYPE>
inline TYPE* ChPtrList<TYPE>::RemoveTail()
{
	return (TYPE*)ChParamList::RemoveTail();
}

template<class TYPE>
CH_UNIX_INLINE ChPosition ChPtrList<TYPE>::InsertBefore( ChPosition pos, const TYPE* pNewVal )
{
	return ChParamList::InsertBefore( pos, (chparam)pNewVal );
}

template<class TYPE>
CH_UNIX_INLINE ChPosition ChPtrList<TYPE>::InsertAfter( ChPosition pos, const TYPE* pNewVal )
{
	return ChParamList::InsertBefore( pos, (chparam)pNewVal );
}

template<class TYPE>
inline ChPosition ChPtrList<TYPE>::Find( const TYPE* pSearchValue,
											ChPosition startAfter ) const
{
	return ChParamList::Find( (chparam)pSearchValue, startAfter );
}


/*----------------------------------------------------------------------------
	ChList template class
----------------------------------------------------------------------------*/

template<class TYPE> class ChList
{
	public:
		ChList();
		virtual ~ChList();

		chint32 GetCount() const;
		bool IsEmpty() const;

		TYPE& GetHead();
		TYPE GetHead() const;
		TYPE& GetTail();
		TYPE GetTail() const;
											/* Get head or tail (and remove
												it) - don't call on empty
												list */
		TYPE RemoveHead();
		TYPE RemoveTail();
											/* Add before head or after tail -
												returns position of new
												element */
		ChPosition AddHead( const TYPE& newVal );
		ChPosition AddTail( const TYPE& newVal );

											/* Add another list of elements
												before head or after tail */
		void AddHead( ChList* pSrcList );
		void AddTail( ChList* pSrcList );
											/* Delete all elements from the
												list */
		void Empty();

		ChPosition GetHeadPosition() const;
		ChPosition GetTailPosition() const;
		TYPE& GetNext( ChPosition& pos );
		TYPE GetNext( ChPosition& pos ) const;
		TYPE& GetPrev( ChPosition& pos );
		TYPE GetPrev( ChPosition& pos ) const;

											/* Get or change an element
												at a given position */
		TYPE& Get( ChPosition pos );
		TYPE Get( ChPosition pos ) const;
		void Set( ChPosition pos, const TYPE& newVal );
		void Remove( ChPosition pos );
											/* Inserting before or after a
												given position */

		ChPosition InsertBefore( ChPosition pos, const TYPE& newVal );
		ChPosition InsertAfter( ChPosition pos, const TYPE& newVal );

		ChPosition Find( const TYPE& searchValue, ChPosition startAfter = 0 ) const;
		ChPosition FindIndex( chint32 lIndex ) const;

	protected:
		ChListNode<TYPE>*	m_pHead;
		ChListNode<TYPE>*	m_pTail;
		chint32				m_lCount;

	protected:
		ChListNode<TYPE>* NewNode( ChListNode<TYPE>* pPrev, ChListNode<TYPE>* pNext );
		void FreeNode( ChListNode<TYPE>* pNode );
};


/*----------------------------------------------------------------------------
	ChList public inline methods
----------------------------------------------------------------------------*/

template<class TYPE>
CH_UNIX_INLINE ChList<TYPE>::ChList() : m_pHead( 0 ), m_pTail( 0 ), m_lCount( 0 )
{
}


template<class TYPE>
CH_UNIX_INLINE ChList<TYPE>::~ChList()
{
	Empty();
	ASSERT( m_lCount == 0 );
}


template<class TYPE>
inline chint32 ChList<TYPE>::GetCount() const { return m_lCount; }

template<class TYPE>
inline bool ChList<TYPE>::IsEmpty() const { return m_lCount == 0; }

template<class TYPE>
inline TYPE& ChList<TYPE>::GetHead()
{
	ASSERT( m_pHead != 0 );
	return m_pHead->item;
}

template<class TYPE>
inline TYPE ChList<TYPE>::GetHead() const
{
	ASSERT( m_pHead != 0 );
	return m_pHead->item;
}

template<class TYPE>
inline TYPE& ChList<TYPE>::GetTail()
{
	ASSERT( m_pTail != 0 );
	return m_pTail->item;
}

template<class TYPE>
inline TYPE ChList<TYPE>::GetTail() const
{
	ASSERT( m_pTail != 0 );
	return m_pTail->item;
}

template<class TYPE>
inline ChPosition ChList<TYPE>::GetHeadPosition() const
{
	return (ChPosition)m_pHead;
}

template<class TYPE>
inline ChPosition ChList<TYPE>::GetTailPosition() const
{
	return (ChPosition)m_pTail;
}

template<class TYPE>
inline TYPE& ChList<TYPE>::GetNext( ChPosition& pos )
{
	ChListNode<TYPE>*	pNode = (ChListNode<TYPE>*)pos;

	#if defined( CH_MSW )
	{
		ASSERT( AfxIsValidAddress( pNode, sizeof( ChListNode<TYPE> ) ) );
	}
	#endif	// defined( CH_MSW )

	pos = (ChPosition)pNode->pNext;
	return pNode->item;
}

template<class TYPE>
inline TYPE ChList<TYPE>::GetNext( ChPosition& pos ) const
{
	ChListNode<TYPE>*		pNode = (ChListNode<TYPE>*)pos;

	#if defined( CH_MSW )
	{
		ASSERT( AfxIsValidAddress( pNode, sizeof( ChListNode<TYPE> ) ) );
	}
	#endif	// defined( CH_MSW )

	pos = (ChPosition)pNode->pNext;
	return pNode->item;
}

template<class TYPE>
inline TYPE& ChList<TYPE>::GetPrev( ChPosition& pos )
{
	ChListNode<TYPE>*		pNode = (ChListNode<TYPE>*)pos;

	#if defined( CH_MSW )
	{
		ASSERT( AfxIsValidAddress( pNode, sizeof( ChListNode<TYPE> ) ) );
	}
	#endif	// defined( CH_MSW )

	pos = (ChPosition)pNode->pPrev;
	return pNode->item;
}

template<class TYPE>
inline TYPE ChList<TYPE>::GetPrev( ChPosition& pos ) const
{
	ChListNode<TYPE>*		pNode = (ChListNode<TYPE>*)pos;

	#if defined( CH_MSW )
	{
		ASSERT( AfxIsValidAddress( pNode, sizeof( ChListNode<TYPE> ) ) );
	}
	#endif	// defined( CH_MSW )

	pos = (ChPosition)pNode->pPrev;
	return pNode->item;
}

template<class TYPE>
inline TYPE& ChList<TYPE>::Get( ChPosition pos )
{
	ChListNode<TYPE>*		pNode = (ChListNode<TYPE>*)pos;

	#if defined( CH_MSW )
	{
		ASSERT( AfxIsValidAddress( pNode, sizeof( ChListNode<TYPE> ) ) );
	}
	#endif	// defined( CH_MSW )

	return pNode->item;
}

template<class TYPE>
inline TYPE ChList<TYPE>::Get( ChPosition pos ) const
{
	ChListNode<TYPE>* pNode = (ChListNode<TYPE>*)pos;

	#if defined( CH_MSW )
	{
		ASSERT( AfxIsValidAddress( pNode, sizeof( ChListNode<TYPE> ) ) );
	}
	#endif	// defined( CH_MSW )

	return pNode->item;
}

template<class TYPE>
inline void ChList<TYPE>::Set( ChPosition pos, const TYPE& newVal )
{
	ChListNode<TYPE>*		pNode = (ChListNode<TYPE>*)pos;

	#if defined( CH_MSW )
	{
		ASSERT( AfxIsValidAddress( pNode, sizeof( ChListNode<TYPE> ) ) );
	}
	#endif	// defined( CH_MSW )

	pNode->item = newVal;
}

template<class TYPE>
CH_UNIX_INLINE void ChList<TYPE>::Empty()
{
	ChListNode<TYPE>*	pOldHead;
											// Destroy elements
	while (pOldHead = m_pHead)
	{										// Call the destructor helper
		ChDestructHelper( &pOldHead->item );

		m_pHead = pOldHead->pNext;
		FreeNode( pOldHead );
	}

	m_lCount = 0;
	m_pHead = m_pTail = 0;
}

template<class TYPE>
CH_UNIX_INLINE ChPosition ChList<TYPE>::AddHead( const TYPE& newVal )
{
	ChListNode<TYPE>*		pNewNode;

	pNewNode = NewNode( 0, m_pHead );
	pNewNode->item = newVal;
	if (m_pHead != 0)
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

template<class TYPE>
CH_UNIX_INLINE ChPosition ChList<TYPE>::AddTail( const TYPE& newVal )
{
	ChListNode<TYPE>*		pNewNode;

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

template<class TYPE>
CH_UNIX_INLINE void ChList<TYPE>::AddHead( ChList* pNewList )
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

template<class TYPE>
CH_UNIX_INLINE void ChList<TYPE>::AddTail( ChList* pNewList )
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

template<class TYPE>
CH_UNIX_INLINE TYPE ChList<TYPE>::RemoveHead()
{
	ASSERT( m_pHead != 0 );				// don't call on empty list

	ChListNode<TYPE>*		pOldNode = m_pHead;
	TYPE		returnValue = pOldNode->item;

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

template<class TYPE>
CH_UNIX_INLINE TYPE ChList<TYPE>::RemoveTail()
{
	ASSERT( m_pTail != 0 );				// don't call on empty list

	ChListNode<TYPE>*	pOldNode = m_pTail;
	TYPE				returnValue = pOldNode->item;

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

template<class TYPE>
CH_UNIX_INLINE ChPosition ChList<TYPE>::InsertBefore( ChPosition pos, const TYPE& newVal )
{
	if (pos == 0)
	{
		return AddHead( newVal );
	}

	ChListNode<TYPE>*		pOldNode = (ChListNode<TYPE>*)pos;
	ChListNode<TYPE>*		pNewNode = NewNode( pOldNode->pPrev, pOldNode );
	
	pNewNode->item = newVal;

	if (pOldNode->pPrev != 0)
	{
		#if defined( CH_MSW )
		{
			ASSERT( AfxIsValidAddress( pOldNode->pPrev, sizeof( ChListNode<TYPE> ) ) );
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

template<class TYPE>
CH_UNIX_INLINE ChPosition ChList<TYPE>::InsertAfter( ChPosition pos, const TYPE& newVal )
{
	ChListNode<TYPE>*		pOldNode;
	ChListNode<TYPE>*		pNewNode;

	if (pos == 0)
	{
		return AddTail( newVal );
	}

	pOldNode = (ChListNode<TYPE>*)pos;

	#if defined( CH_MSW )
	{
		ASSERT( AfxIsValidAddress( pOldNode, sizeof( ChListNode<TYPE> ) ) );
	}
	#endif	// defined( CH_MSW )

	pNewNode = NewNode( pOldNode, pOldNode->pNext );
	pNewNode->item = newVal;

	if (pOldNode->pNext != 0)
	{
		#if defined( CH_MSW )
		{
			ASSERT( AfxIsValidAddress( pOldNode->pNext,
										sizeof( ChListNode<TYPE> ) ) );
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

template<class TYPE>
CH_UNIX_INLINE void ChList<TYPE>::Remove( ChPosition pos )
{
	ChListNode<TYPE>*		pOldNode;

	pOldNode = (ChListNode<TYPE>*)pos;

	#if defined( CH_MSW )
	{
		ASSERT( AfxIsValidAddress( pOldNode, sizeof( ChListNode<TYPE> ) ) );
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
										sizeof( ChListNode<TYPE> ) ) );
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
										sizeof( ChListNode<TYPE> ) ) );
		}
		#endif	// defined( CH_MSW )

		pOldNode->pNext->pPrev = pOldNode->pPrev;
	}

	FreeNode( pOldNode );
}

template<class TYPE>
CH_UNIX_INLINE ChPosition ChList<TYPE>::FindIndex( chint32 lIndex ) const
{
	ChListNode<TYPE>*		pNode;

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
			ASSERT( AfxIsValidAddress( pNode, sizeof( ChListNode<TYPE> ) ) );
		}
		#endif	// defined( CH_MSW )

		pNode = pNode->pNext;
	}

	return (ChPosition)pNode;
}

template<class TYPE>
CH_UNIX_INLINE ChPosition ChList<TYPE>::Find( const TYPE& searchValue,
								ChPosition startAfter ) const
{
	ChListNode<TYPE>*		pNode;

	pNode = (ChListNode<TYPE>*)startAfter;
	if (pNode == 0)
	{
		pNode = m_pHead;
	}
	else
	{
		#if defined( CH_MSW )
		{
			ASSERT( AfxIsValidAddress( pNode, sizeof( ChListNode<TYPE> ) ) );
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

template<class TYPE>
CH_UNIX_INLINE ChListNode<TYPE>* ChList<TYPE>::NewNode( ChListNode<TYPE>* pPrev,
									ChListNode<TYPE>* pNext )
{
	ChListNode<TYPE>*	pNode;
											// Create the new node
	pNode = new ChListNode<TYPE>;
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

template<class TYPE>
CH_UNIX_INLINE void ChList<TYPE>::FreeNode( ChListNode<TYPE>* pNode )
{
	delete pNode;
											// Decrement the node count
	m_lCount--;
	ASSERT( m_lCount >= 0 );
}

#endif  // !defined( NO_TEMPLATES )

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR    
#endif

#endif	// !defined( _CHLIST_H )

// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
