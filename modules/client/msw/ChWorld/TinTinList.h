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

	TinTin class definitions.  Originally modified from TinTin++,
	(T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t, originally coded by
	Peter Unold 1992.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( TINTINLIST_H )
#define TINTINLIST_H

#include <ChList.h>


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

											// Search and insert modes:
#define ALPHA					1
#define PRIORITY				0


/*----------------------------------------------------------------------------
	Forward class declarations
----------------------------------------------------------------------------*/

class TinTin;
class TinTinList;


/*----------------------------------------------------------------------------
	TinTinListNode class
----------------------------------------------------------------------------*/

class TinTinListNode
{
	friend class TinTinList;

	public:
		TinTinListNode();
		TinTinListNode( const ChString& strLeft, const ChString& strRight,
						const ChString& strPriority );
		~TinTinListNode();

		inline TinTinListNode* GetNext() { return m_pNext; }
		inline const ChString& GetLeft() { return m_strLeft; }
		inline const ChString& GetRight() { return m_strRight; }
		inline const ChString& GetPriority() { return m_strPriority; }

	protected:
		TinTinListNode*	m_pNext;

		ChString			m_strLeft;
		ChString			m_strRight;
		ChString			m_strPriority;
};


/*----------------------------------------------------------------------------
	TinTinList class
----------------------------------------------------------------------------*/

class TinTinList
{
	public:
		TinTinList( int iMode = ALPHA );
		~TinTinList();

		inline TinTinListNode* GetTop() { return m_pTop; }

		TinTinList& TinTinList::operator=( TinTinList& src );

		void AddNode( const ChString& strLeft, const ChString& strRight,
						const ChString& strPriority );
		void InsertNode( const ChString& strLeft, const ChString& strRight,
							const ChString& strPriority );
		void DeleteNode( TinTinListNode* pDelete );
		int GetCount();
		void Empty();

		TinTinListNode* Search( const ChString& strText );
		TinTinListNode* SearchBegin( const ChString& strText );
		TinTinListNode* SearchWithWildchars( const ChString& strText );
		TinTinListNode* SearchWithWildchars( const ChString& strText,
												TinTinListNode* pStart );

		void ShowList( TinTin* pTinTin );
		void ShowNode(  TinTin* pTinTin, TinTinListNode* pNode );

	protected:
		inline void InsertAfter( TinTinListNode* pNode,
									TinTinListNode* pNew )
						{
							TinTinListNode* pNext;

							if (pNode)
							{
								pNext = pNode->GetNext();
								pNode->m_pNext = pNew;
							}
							else
							{				// Insert as first node

								pNext = m_pTop;
								m_pTop = pNew;
							}

							pNew->m_pNext = pNext;
						}

	protected:
		int					m_iListMode;
		TinTinListNode*		m_pTop;
};


#endif	// !defined( TINTINLIST_H )

// $Log$
