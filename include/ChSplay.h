// -*- mode: c++; tab-width: 4 -*-
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

    This template class implements a splay tree with arbitrary key and data.
    The key class must have "<" and "==" defined. Both key and data must
    have copy constructors.

----------------------------------------------------------------------------*/

#if !defined( _CHSPLAY_H )
#define _CHSPLAY_H  

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif

#if defined( CH_UNIX )
	#define CH_UNIX_INLINE inline
#else
	#define CH_UNIX_INLINE
#endif // defined( CH_UNIX )


/*----------------------------------------------------------------------------
	class ChParamSplayNode
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChParamSplayNode
{
	public:
		chparam				key;
		chparam				data;
		ChParamSplayNode*	left;
		ChParamSplayNode*	right;

		~ChParamSplayNode()
				{
					if (left)
						delete left;
					if (right)
						delete right;
				}

		ChParamSplayNode( chparam inKey, chparam inData ) :
				key( inKey ), data( inData ), left( 0 ), right( 0 ) {}

		ChParamSplayNode( const ChParamSplayNode& source ) :
				key( source.key ), data( source.data ), left( 0 ), right( 0 )
				{
					if (source.left)
						left = new ChParamSplayNode( *source.left );
					if (source.right)
						right = new ChParamSplayNode( *source.right );
				}

		ChParamSplayNode() : left( 0 ), right( 0 ) {}

		ChParamSplayNode* Splay( chparam key );
		ChParamSplayNode* SplayLeft();
};


/*----------------------------------------------------------------------------
	class ChParamSplayVisitor2
----------------------------------------------------------------------------*/

class ChParamSplayVisitor2
{
	public:
		virtual bool Visit( chparam key, chparam data ) = 0;

		virtual void Start() {}
		virtual void Stop() {}
};


#if !defined( NO_TEMPLATES )

/*----------------------------------------------------------------------------
	Template helper functions -- These may be overridden
----------------------------------------------------------------------------*/

template<class TYPE>
inline void ChPtrSplayDestruct( TYPE* pItem )
{
}

#endif // !defined( NO_TEMPLATES )

/*----------------------------------------------------------------------------
	class ChParamSplay
----------------------------------------------------------------------------*/

class ChParamSplayNode;

class CH_EXPORT_CLASS ChParamSplay
{
	public:
		ChParamSplay() : root( 0 ), m_boolInTraverse( false ) {}
		virtual ~ChParamSplay();

		bool IsEmpty();

		chparam* Find( chparam key );
		ChParamSplay& Insert( chparam key, chparam data );
		ChParamSplay& Delete( chparam key );
		const ChParamSplay& Infix( ChParamSplayVisitor2& operation ) const;
		ChParamSplay& Infix( ChParamSplayVisitor2& operation );
		ChParamSplay& Erase();
		ChParamSplay( const ChParamSplay &inTree );
		ChParamSplay& operator=( const ChParamSplay &source );
		ChParamSplay& operator+=( const ChParamSplay &source );

	protected:
		chparam GetTopKey();

	private:
		void Infix( const ChParamSplayNode* node,
					ChParamSplayVisitor2& operation ) const;
		chparam* BoringFind( chparam key ) const;

	protected:
		ChParamSplayNode*	root;
		bool				m_boolInTraverse;

	private:
		class ChParamSplayVisitor2Addin : public ChParamSplayVisitor2
		{
			public:
				ChParamSplay&   target;

				bool Visit( chparam key, chparam data )
						{
							target.Insert( key, data );
						  
						  	return true;
						}

				ChParamSplayVisitor2Addin( ChParamSplay &inTarget ) :
						target( inTarget ) {}
  		};

};

 
#if !defined( NO_TEMPLATES )
 
/*----------------------------------------------------------------------------
	class ChPtrSplayVisitor2
----------------------------------------------------------------------------*/

template <class TYPE> class ChPtrSplayVisitor2
{
	public:
		virtual bool Visit( chparam key, const TYPE* data ) = 0;

		virtual void Start() {}
		virtual void Stop() {}
};


/*----------------------------------------------------------------------------
	class ChPtrSplay
----------------------------------------------------------------------------*/

template <class TYPE> class ChPtrSplay : public ChParamSplay
{
	public:
		ChPtrSplay() {}
		virtual ~ChPtrSplay();

		TYPE* FindValue( chparam key );
		TYPE** Find( chparam key );
		ChPtrSplay& Insert( chparam key, TYPE* data );
		ChPtrSplay& Delete( chparam key );
		const ChPtrSplay& Infix( ChPtrSplayVisitor2<TYPE>& operation ) const;
		ChPtrSplay& Infix( ChPtrSplayVisitor2<TYPE>& operation );

	private:
		void Infix( const ChParamSplayNode* node,
					ChPtrSplayVisitor2<TYPE>& operation ) const;
};


template <class TYPE>
CH_UNIX_INLINE ChPtrSplay<TYPE>::~ChPtrSplay()
{
											// Delete all of the items
	while (!IsEmpty())
	{
		chparam		key = GetTopKey();

		Delete( key );
	}
}


template <class TYPE>
CH_UNIX_INLINE TYPE* ChPtrSplay<TYPE>::FindValue( chparam key )
{
	TYPE**	ppData = (TYPE**)ChParamSplay::Find( key );

	if (ppData)
	{
		return *ppData;
	}
	else
	{
		return 0;
	}
}


template <class TYPE>
CH_UNIX_INLINE TYPE** ChPtrSplay<TYPE>::Find( chparam key )
{
	return (TYPE**)ChParamSplay::Find( key );
}


template <class TYPE>
CH_UNIX_INLINE ChPtrSplay<TYPE>& ChPtrSplay<TYPE>::Insert( chparam key,
															TYPE* data )
{
	ChParamSplay::Insert( key, (chparam)data );
	return *this;
}


template <class TYPE>
CH_UNIX_INLINE ChPtrSplay<TYPE>& ChPtrSplay<TYPE>::Delete( chparam key )
{
	TYPE**	ppData = Find( key );
	TYPE*	pData;

	ASSERT( 0 != ppData );

	pData = *ppData;

	ChParamSplay::Delete( key );
	ChPtrSplayDestruct( pData );

	return *this;
}


template <class TYPE>
CH_UNIX_INLINE const ChPtrSplay<TYPE>& ChPtrSplay<TYPE>::
Infix( ChPtrSplayVisitor2<TYPE>& operation ) const
{
	bool	boolOldInTraverse = m_boolInTraverse;

	operation.Start();
	((ChPtrSplay<TYPE>*)this)->m_boolInTraverse = true;
	Infix( root, operation );
	((ChPtrSplay<TYPE>*)this)->m_boolInTraverse = boolOldInTraverse;
	operation.Stop();

	return *this;
}

template <class TYPE>
CH_UNIX_INLINE ChPtrSplay<TYPE>& ChPtrSplay<TYPE>::
Infix( ChPtrSplayVisitor2<TYPE>& operation )
{
	bool	boolOldInTraverse = m_boolInTraverse;

	operation.Start();
	m_boolInTraverse = true;
	Infix( root, operation );
	m_boolInTraverse = boolOldInTraverse;
	operation.Stop();

	return *this;
}


template <class TYPE>
CH_UNIX_INLINE void ChPtrSplay<TYPE>::Infix( const ChParamSplayNode *node,
											ChPtrSplayVisitor2<TYPE> &operation ) const
{
	if (node)
	{
		Infix( node->left, operation );
		operation.Visit( node->key, (TYPE*)node->data );
		Infix( node->right, operation );
	}
}


/*----------------------------------------------------------------------------
	template class ChSplayNode
----------------------------------------------------------------------------*/

template <class K,class D> class ChSplayNode
{
	public:
		typedef ChSplayNode<K,D> TSelf;

		K		key;
		D		data;
		TSelf	*left;
		TSelf	*right;

		~ChSplayNode()
				{
					if (left)
						delete left;
					if (right)
						delete right;
				};

		ChSplayNode( const K& inKey, const D& inData ) :
				key( inKey ), data( inData ), left( 0 ), right( 0 ) {};

		ChSplayNode( const ChSplayNode& source ) :
				key( source.key ), data( source.data ), left( 0 ), right( 0 )
				{
					if (source.left)
						left = new ChSplayNode( *source.left );
					if (source.right)
						right = new ChSplayNode( *source.right );
				};

		ChSplayNode() : left(0), right(0) {};

		ChSplayNode* Splay( const K& key );
		ChSplayNode* SplayLeft();
};


template <class K,class D> class ChVisitor2
{
	public:
		virtual bool Visit( const K& key, const D& data ) = 0;

		virtual void Start() {};
		virtual void Stop() {};
};


/*----------------------------------------------------------------------------
	template class ChSplay
----------------------------------------------------------------------------*/

template <class K,class D> class ChSplay
{
	private:
		class ChVisitor2Addin : public ChVisitor2<K,D>
		{
			public:
				ChSplay<K,D>&   target;

				bool Visit( const K& key, const D& data )
						{ target.Insert( key, data );
						  return 1;
						};
				ChVisitor2Addin( ChSplay<K,D> &inTarget ) : target( inTarget )
						{};
  		};

	public:
		typedef ChSplayNode<K,D>	TNode;

	private:
		TNode						*root;
		bool						m_boolInTraverse;

		void Infix( const ChSplayNode<K,D> *node,
					ChVisitor2<K,D> &operation ) const;
		D* BoringFind( const K &key ) const;

	public:
		ChSplay() : root( 0 ), m_boolInTraverse( false ) {};
		~ChSplay();

		D* Find( const K& );
		// D& operator( const K& );
		ChSplay& Insert( const K&, const D& );
		ChSplay& Delete( const K& );
		const ChSplay& Infix( ChVisitor2<K,D> &operation ) const;
		ChSplay& Infix( ChVisitor2<K,D> &operation );
		ChSplay& Erase();
		K* GetLastKey();	// UE
		ChSplay( const ChSplay &inTree );
		ChSplay& operator=( const ChSplay &source );
		ChSplay& operator+=( const ChSplay &source );
};

template <class K,class D>
CH_UNIX_INLINE D* ChSplay<K,D>::Find( const K &key )
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

template <class K,class D>
CH_UNIX_INLINE D* ChSplay<K,D>::BoringFind( const K &key ) const
{
	ChSplayNode<K,D>*	pNode = root;

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

// template <class K,class D> D& ChSplay<K,D>::operator[](const K& key)
// {
//	return *Find(key);
// }

template <class K,class D>
CH_UNIX_INLINE ChSplayNode<K,D>* ChSplayNode<K,D>::Splay( const K &key )
{
	ChSplayNode<K,D>	*t = this;
											/* Simple top down splay, not
												requiring i to be in the tree
												t.  What it does is described
												above. */
	ChSplayNode<K,D>	N, *l, *r, *y;

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

template <class K,class D>
CH_UNIX_INLINE ChSplayNode<K,D>* ChSplayNode<K,D>::SplayLeft()
{
	ChSplayNode<K,D>	*t = this;

											/* Simple top down splay, not
												requiring i to be in the tree
												t.  What it does is described
												above. */

	ChSplayNode<K,D>	N, *l, *r, *y;

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

template <class K,class D>
CH_UNIX_INLINE ChSplay<K,D>& ChSplay<K,D>::Insert( const K& key,const D& data )
{
	TNode	*n = new TNode( key,data );

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

template <class K,class D>
CH_UNIX_INLINE ChSplay<K,D>& ChSplay<K,D>::Delete( const K &key )
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
	TNode	*newroot;

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

template <class K,class D>
CH_UNIX_INLINE const ChSplay<K,D>& ChSplay<K,D>::Infix( ChVisitor2<K,D> &operation ) const
{
	bool	boolOldInTraverse = m_boolInTraverse;

	operation.Start();
	((ChSplay<K,D>*)this)->m_boolInTraverse = true;
	Infix( root, operation );
	((ChSplay<K,D>*)this)->m_boolInTraverse = boolOldInTraverse;
	operation.Stop();

	return *this;
}

template <class K,class D>
CH_UNIX_INLINE ChSplay<K,D>& ChSplay<K,D>::Infix( ChVisitor2<K,D> &operation )
{
	bool	boolOldInTraverse = m_boolInTraverse;

	operation.Start();
	m_boolInTraverse = true;
	Infix( root, operation );
	m_boolInTraverse = boolOldInTraverse;
	operation.Stop();

	return *this;
}

template <class K,class D>
CH_UNIX_INLINE void ChSplay<K,D>::Infix( const ChSplayNode<K,D> *node,
							ChVisitor2<K,D> &operation ) const
{
	if (node)
	{
		Infix( node->left, operation );
		operation.Visit( node->key, node->data );
		Infix( node->right, operation );
	}
}

template <class K,class D>
CH_UNIX_INLINE ChSplay<K,D>& ChSplay<K,D>::Erase()
{
	delete root;
	root = 0;

	return *this;
}

template <class K,class D>
CH_UNIX_INLINE ChSplay<K,D>::~ChSplay()
{
	delete root;
}

template <class K,class D>
CH_UNIX_INLINE ChSplay<K,D>::ChSplay( const ChSplay &inTree )
{
	root = new TNode( *inTree.root );
}

template <class K,class D>
CH_UNIX_INLINE ChSplay<K,D>& ChSplay<K,D>::operator=( const ChSplay &source )
{
	Erase();
	if (source.root)
	{
		root = new TNode( *source.root );
	}
	else
	{
		root = 0;
	}

	return *this;
}

template <class K,class D>
CH_UNIX_INLINE ChSplay<K,D>& ChSplay<K,D>::operator+=( const ChSplay &source )
{
	ChVisitor2Addin		op( *this );

	return Infix( op );
} 
#endif // !defined NO_TEMPLATES

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR    
#endif

#endif // !defined( _CHSPLAY_H )
