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

	TinTin class linked list methods.  Originally modified from TinTin++,
	(T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t, originally coded by
	Peter Unold 1992.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <ctype.h>
#include <signal.h>

#include "TinTin.h"



/*----------------------------------------------------------------------------
	TinTinListNode class
----------------------------------------------------------------------------*/

TinTinListNode::TinTinListNode() :
					m_pNext( 0 )
{
}


TinTinListNode::TinTinListNode( const ChString& strLeft, const ChString& strRight,
								const ChString& strPriority ) :
					m_pNext( 0 ),
					m_strLeft( strLeft ),
					m_strRight( strRight ),
					m_strPriority( strPriority )
{
}


TinTinListNode::~TinTinListNode()
{
}


/*----------------------------------------------------------------------------
	TinTinList class
----------------------------------------------------------------------------*/

TinTinList::TinTinList( int iMode ) :
				m_iListMode( iMode ),
				m_pTop( 0 )
{
}

	
TinTinList::~TinTinList()
{
	Empty();
}

	
TinTinList& TinTinList::operator=( TinTinList& src )
{
	TinTinListNode*		pTop = src.GetTop();

	Empty();

	if (pTop)
	{
		while (pTop)
		{
			InsertNode( pTop->GetLeft(), pTop->GetRight(),
							pTop->GetPriority() );

			pTop = pTop->GetNext();
		}
	}

	return *this;
}


void TinTinList::AddNode( const ChString& strLeft, const ChString& strRight,
							const ChString& strPriority )
{
	TinTinListNode* 	pNew;
	TinTinListNode* 	pTop = GetTop();

	pNew = new TinTinListNode( strLeft, strRight, strPriority );

	if (0 == pTop)
	{										// First node in list
		m_pTop = pNew;
	}
	else
	{
		while (0 != pTop->GetNext())
		{
			pTop = pTop->GetNext();
		}

		pTop->m_pNext = pNew;
	}
}


/*----------------------------------------------------------------------------
	TinTinList::InsertNode
					Create a node containing the strLeft, strRight, and
					strPriority fields and stuff it into the list - in
					lexicographical order, or by numerical priority
					(dependent on mode)
----------------------------------------------------------------------------*/

void TinTinList::InsertNode( const ChString& strLeft, const ChString& strRight,
								const ChString& strPriority )
{
	TinTinListNode* 	pNew;
	TinTinListNode* 	pTop = GetTop();
	TinTinListNode* 	pLast = 0;

	pNew = new TinTinListNode( strLeft, strRight, strPriority );

	if (0 == pTop)
	{										/* List was empty -- make the new
												node the top node */
		m_pTop = pNew;
	}
	else
	{
		switch (m_iListMode)
		{
			case PRIORITY:
			{
				while (pTop)
				{
					if (strcmp( strPriority, pTop->GetPriority() ) < 0)
					{
						InsertAfter( pLast, pNew );
						return;
					}
					else if (strcmp( strPriority, pTop->GetPriority() ) == 0)
					{
											/* Priorities being equal, insert
												the string in alphabetical
												order */

						while (pTop &&
								(strcmp( strPriority, pTop->GetPriority() ) == 0))
						{
							if (strcmp( strLeft, pTop->GetLeft() ) <= 0)
							{
								InsertAfter( pLast, pNew );
								return;
							}

							pLast = pTop;
							pTop = pTop->GetNext();
						}

						InsertAfter( pLast, pNew );
						return;
					}

					pLast = pTop;
					pTop = pTop->GetNext();
				}

				InsertAfter( pLast, pNew );
				break;
			}

			case ALPHA:
			{
				while (pTop)
				{
					if (strcmp( strLeft, pTop->GetLeft() ) <= 0)
					{
						InsertAfter( pLast, pNew );
						return;
					}

					pLast = pTop;
					pTop = pTop->GetNext();
				}

				InsertAfter( pLast, pNew );
				break;
			}
		}
	}
}


void TinTinList::DeleteNode( TinTinListNode* pDelete )
{
	TinTinListNode* 	pTop = GetTop();

	if (pTop)
	{
		if (pTop == pDelete)
		{									// First node in list
			m_pTop = pTop->GetNext();
			delete pDelete;
		}
		else
		{
			TinTinListNode*	pLast = pTop;

			while (pTop = pTop->GetNext())
			{
				if (pTop == pDelete)
				{
					pLast->m_pNext = pTop->GetNext();
					delete pTop;
					return;
				}
	
				pLast = pTop;
			}
		}
	}
}


int TinTinList::GetCount()
{ 
	int				iCount = 0;
	TinTinListNode*	pTop = GetTop();

	while (pTop)
	{
		iCount++;
		pTop = pTop->GetNext();
	}

	return iCount;
}


/*----------------------------------------------------------------------------
	TinTinList::Empty
					Deletes every node in the list.
----------------------------------------------------------------------------*/

void TinTinList::Empty()
{
	TinTinListNode* 	pTop = GetTop();

	while (pTop)
	{
		TinTinListNode* 	pNext = pTop->GetNext();

		delete pTop;
		pTop = pNext;
	}

	m_pTop = 0;
}


/*----------------------------------------------------------------------------
	TinTinList::Search
					Search for a node that has strText in the left value.
					Return: ptr to node on success / 0 on failure.
----------------------------------------------------------------------------*/

TinTinListNode* TinTinList::Search( const ChString& strText )
{
	TinTinListNode*	pTop = GetTop();

	while (pTop)
	{
		if (strcmp( pTop->GetLeft(), strText ) == 0)
		{
			return pTop;
		}

		pTop = pTop->GetNext();
	}

	return 0;
}

	
/*----------------------------------------------------------------------------
	TinTinList::SearchBegin
					Search for a node that has strText as a beginning.
					Return: ptr to node on success / 0 on failure.
----------------------------------------------------------------------------*/

TinTinListNode* TinTinList::SearchBegin( const ChString& strText )
{
	const char*		pstrText = strText;
	int				iTextLen = strText.GetLength();
	int				iCmp;
	TinTinListNode*	pTop = GetTop();

	if (0 == pTop)
	{
		return 0;
	}

	switch (m_iListMode)
	{
		case PRIORITY:
		{
			while (pTop)
			{
				iCmp = strncmp( pTop->GetLeft(), pstrText, iTextLen );

				if ((iCmp == 0) &&
					((*((const char*)pTop->GetLeft() + iTextLen) == ' ') ||
						(*((const char*)pTop->GetLeft() + iTextLen) == '\0')))
				{
					return pTop;
				}

				pTop = pTop->GetNext();
			}
			return 0;
		}

		case ALPHA:
		{
			while (pTop)
			{
				iCmp = strncmp( pTop->GetLeft(), pstrText, iTextLen );

				if ((iCmp == 0) &&
					((*((const char*)pTop->GetLeft() + iTextLen) == ' ') ||
						(*((const char*)pTop->GetLeft() + iTextLen) == '\0')))
				{
					return pTop;
				}
				else if (iCmp > 0)
				{
					return 0;
				}

				pTop = pTop->GetNext();
			}
			return 0;
		}

		default:
		{
			return 0;
		}
	}
}


TinTinListNode* TinTinList::SearchWithWildchars( const ChString& strText )
{
	if (0 != GetTop())
	{
		return SearchWithWildchars( strText, GetTop() );
	}
	else
	{
		return 0;
	}
}


TinTinListNode* TinTinList::SearchWithWildchars( const ChString& strText,
													TinTinListNode* pStart )
{
	while (pStart)
	{
		if (::Match( strText, pStart->GetLeft() ))
		{
			return pStart;
		}

		pStart = pStart->GetNext();
	}

	return 0;
}


void TinTinList::ShowList( TinTin* pTinTin )
{
	TinTinListNode*	pTop = GetTop();

	if (pTop)
	{
		while (pTop)
		{
			ShowNode( pTinTin, pTop );
			pTop = pTop->GetNext();
		}
	}
}


void TinTinList::ShowNode( TinTin* pTinTin, TinTinListNode* pNode )
{
	ChString		strTemp;

	strTemp.Format( "#    {%s} = {%s}", (const char*)pNode->GetLeft(),
										(const char*)pNode->GetRight() );
	pTinTin->Message( strTemp );
}

// $Log$
