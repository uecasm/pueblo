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

	Contains the interface for the ChWorldInfo class.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHWLIST_H )
#define _CHWLIST_H

#include <ChList.h>
#include <ChReg.h>           

#if defined( NO_TEMPLATES )
#include "templcls\ChMdPLst.h"
#include "templcls\ChMdPLst.inl"
#endif


/*----------------------------------------------------------------------------
	ChWorldList destruct helper
----------------------------------------------------------------------------*/

static void ChDestructHelper( ChWorldInfo* pItem )
{
	delete pItem;
}


/*----------------------------------------------------------------------------
	ChWorldList class
----------------------------------------------------------------------------*/

class ChWorldList
{
	public:
		ChWorldList();
		virtual ~ChWorldList();

		inline bool IsDirty() { return m_boolDirty; }

		void Load();
		void Store();

		void Add( const ChString& strName, const ChString& strDesc,
						const ChString& strHost, chint16 sPort,
						const ChWorldType& type, ChLoginType login,
						const ChString& strUsername, const ChString& strPassword,
						const ChString& strHomePage );
		void Add( const ChWorldInfo& info );
		void Remove( const ChString& strName, const ChString& strUsername = "" );

		inline ChPosition GetHead()
						{
							return m_list.GetHeadPosition();
						}
		inline void GetNext( ChPosition& pos )
						{
							m_list.GetNext( pos );
						}
		inline ChWorldInfo* GetData( ChPosition pos )
						{
							return m_list.Get( pos );
						}

		ChWorldInfo* FindName( const ChString& strName,
								const ChString& strUsername = "" );

	protected:
		ChPosition FindNamePos( const ChString& strName,
								const ChString& strUsername );

	private: 
		#if !defined( NO_TEMPLATES )
		ChPtrList<ChWorldInfo>	m_list;
		#else 
		ChWorldInfoList			m_list;
		#endif
		bool					m_boolDirty;
		ChRegistry*				m_pReg;
};


#endif	// !defined( _CHWLIST_H )

// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
