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

	This file consists of the implementation of the splay tree classes.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"
#ifdef CH_UNIX
#include <ChTypes.h>
#endif // CH_UNIX
#include <ChSplay.h>

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	class ChParamSplayNode
----------------------------------------------------------------------------*/

ChParamSplayNode* ChParamSplayNode::Splay( chparam key )
{
	ChParamSplayNode*		t = this;
											/* Simple top down splay, not
												requiring i to be in the tree
												t.  What it does is described
												above. */
	ChParamSplayNode	N, *l, *r, *y;

	if (!t)
	{
		return t;
	}
											/* N.left = N.right = 0; (already
												initialized) */
	l = r = &N;

	for (;;)
	{
		if (key < t->key)
		{
			if (!t->left)
			{
				break;
			}

			if (key < t->left->key)
			{
				y = t->left;				// Rotate right
				t->left = y->right;
				y->right = t;
				t = y;

				if (!t->left)
				{
					break;
				}
			}

			r->left = t;					// Link right
			r = t;
			t = t->left;
		}
		else if (t->key < key)
		{
			if (!t->right)
			{
				break;
			}

			if (key > t->right->key)
			{
				y = t->right;				// Rotate left
				t->right = y->left;
				y->left = t;
				t = y;

				if (!t->right)
				{
					break;
				}
			}

			l->right = t;					// Link left
			l = t;
			t = t->right;
		}
		else
		{
			break;
		}
	}

	l->right = t->left;						// Assemble
	r->left = t->right;
	t->left = N.right;
	t->right = N.left;
	N.right = N.left = 0;

	return t;
}


/*----------------------------------------------------------------------------
	This is an optimization based on the key being always greater than
	anything in the tree.  (Used by delete.)
----------------------------------------------------------------------------*/

ChParamSplayNode* ChParamSplayNode::SplayLeft()
{
	ChParamSplayNode	*t = this;
											/* Simple top down splay, not
												requiring i to be in the tree
												t.  What it does is described
												above. */

	ChParamSplayNode	N, *l, *r, *y;

	if (!t)
	{
		return t;
	}
											/* N.left = N.right = 0; (already
												initialized) */
	l = r = &N;

	for (;;)
	{
		if (!t->right)
		{
			break;
		}

		y = t->right;						// Rotate left
		t->right = y->left;
		y->left = t;
		t = y;

		if (!t->right)
		{
			break;
		}

		l->right = t;						// Link left
		l = t;
		t = t->right;
	}

	l->right = t->left;						// Assemble
	r->left = t->right;
	t->left = N.right;
	t->right = N.left;
	N.right = N.left = 0;

	return t;
}


/*----------------------------------------------------------------------------
	class ChParamSplay
----------------------------------------------------------------------------*/

ChParamSplay::ChParamSplay( const ChParamSplay &inTree )
{
	root = new ChParamSplayNode( *inTree.root );
}


ChParamSplay::~ChParamSplay()
{
	delete root;
}


bool ChParamSplay::IsEmpty()
{
	return 0 == root;
}


chparam ChParamSplay::GetTopKey()
{
	ASSERT( 0 != root );

	return root->key;
}


chparam* ChParamSplay::Find( chparam key )
{
	if (m_boolInTraverse)
	{
		return BoringFind( key );
	}

	root = root->Splay( key );
	if (!root)
	{
		return 0;
	}

	if (root->key == key)
	{
		return &root->data;
	}
	else
	{
		return 0;
	}
}


chparam* ChParamSplay::BoringFind( chparam key ) const
{
	ChParamSplayNode*	pNode = root;

	while (true)
	{
		if (key < pNode->key)
		{
			if (pNode->left)
			{
				pNode = pNode->left;
			}
			else
			{
				return 0;
			}
		}
		else if (key > pNode->key)
		{
			if (pNode->right)
			{
				pNode = pNode->right;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			return &pNode->data;
		}
	}
}


ChParamSplay& ChParamSplay::Insert( chparam key, chparam data )
{
	ChParamSplayNode*		n = new ChParamSplayNode( key,data );

	ASSERT( !m_boolInTraverse );

	if (root)
	{
		Find( key );
											/* Now the root is where we want
												to Insert */
		if (n->key < root->key)
		{
			n->right = root;
			n->left = root->left;
			root->left = 0;
		}
		else
		{
			n->left = root;
			n->right = root->right;
			root->right = 0;
		}
	}

	root = n;

	return *this;
}


ChParamSplay& ChParamSplay::Delete( chparam key )
{
	ASSERT( !m_boolInTraverse );

	if (!root)
	{
		return *this;
	}

	root = root->Splay( key );

	if (root->key != key)
	{
		return *this;						// Should raise exception!
	}
											// Now we must delete the root.
	ChParamSplayNode	*newroot;

	if (!root->left)
	{
		newroot = root->right;
	}
	else
	{
		newroot = root->left->Splay(key);	// not efficient! (no compares needed)
		newroot->right = root->right;
	}

	root->right = root->left = 0;
	delete root;

	root = newroot;

	return *this;
}


const ChParamSplay& ChParamSplay::Infix( ChParamSplayVisitor2& operation ) const
{
	bool	boolOldInTraverse = m_boolInTraverse;

	operation.Start();
	((ChParamSplay*)this)->m_boolInTraverse = true;
	Infix( root, operation );
	((ChParamSplay*)this)->m_boolInTraverse = boolOldInTraverse;
	operation.Stop();

	return *this;
}


ChParamSplay& ChParamSplay::Infix( ChParamSplayVisitor2& operation )
{
	bool	boolOldInTraverse = m_boolInTraverse;

	operation.Start();
	m_boolInTraverse = true;
	Infix( root, operation );
	m_boolInTraverse = boolOldInTraverse;
	operation.Stop();

	return *this;
}


void ChParamSplay::Infix( const ChParamSplayNode *node, ChParamSplayVisitor2& operation ) const
{
	if (node)
	{
		Infix( node->left, operation );
		operation.Visit( node->key, node->data );
		Infix( node->right, operation );
	}
}


ChParamSplay& ChParamSplay::Erase()
{
	delete root;
	root = 0;

	return *this;
}


ChParamSplay& ChParamSplay::operator=( const ChParamSplay &source )
{
	Erase();
	if (source.root)
	{
		root = new ChParamSplayNode( *source.root );
	}
	else
	{
		root = 0;
	}

	return *this;
}


ChParamSplay& ChParamSplay::operator+=( const ChParamSplay &source )
{
	ChParamSplayVisitor2Addin	op( *this );

	return Infix( op );
}

// $Log$
