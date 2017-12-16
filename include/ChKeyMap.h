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

	This file consists of the interface for the ChKeyMap class, use to
	map keys to functionality.

----------------------------------------------------------------------------*/

#if (!defined( _CHKEYMAP_H ))
#define _CHKEYMAP_H

#include <ChArch.h>
#include <ChList.h>


/*----------------------------------------------------------------------------
	ChKeyMapItem class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChKeyMapItem : public ChStreamable
{
	public:
		ChKeyMapItem();
		ChKeyMapItem( chuint32 luKey, chflag32 flModifiers,
						const ChString& strName, chparam userData );

		virtual ~ChKeyMapItem();

		inline chuint32 GetKey() const { return m_luKey; }
		inline chflag32 GetModifiers() const { return m_flModifiers; }
		inline const ChString& GetName() const { return m_strName; }
		inline chparam GetUserData() const { return m_userData; }

		inline void GetData( chuint32& luKey, chflag32& flModifiers,
								ChString& strName, chparam& userData ) const
				{
					luKey = m_luKey;
					flModifiers = m_flModifiers;
					strName = m_strName;
					userData = m_userData;
				}

		inline void SetKey( chuint32 luKey )
				{
					m_luKey = luKey;
				}
		inline void SetModifiers( chflag32 flModifiers )
				{
					m_flModifiers = flModifiers;
				}
		inline void SetName( ChString strName )
				{
					m_strName = strName;
				}
		inline void SetUserData( chparam userData )
				{
					m_userData = userData;
				}

		inline void SetData( chuint32 luKey, chflag32 flModifiers,
								ChString strName, chparam userData )
				{
					m_luKey = luKey;
					m_flModifiers = flModifiers;
					m_strName = strName;
					m_userData = userData;
				}
											// Overrides

		virtual void Serialize( ChArchive& ar );

	protected:
		chuint32	m_luKey;
		chflag32	m_flModifiers;
		ChString		m_strName;
		chparam		m_userData;
};


/*----------------------------------------------------------------------------
	ChKeyMap class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChKeyMap : public ChStreamable
{
	public:
		ChKeyMap();
		virtual ~ChKeyMap();

		void AddItem( chuint32 luKey, chflag32 flModifiers,
						const ChString& strName, chparam userData );
		void AddItem( const ChKeyMapItem& item );
		ChPosition FindItem( const ChString& strName ) const;
		ChPosition FindItem( chuint32 luKey, chflag32 flModifiers ) const;
		ChKeyMapItem* GetItem( ChPosition pos ) const;
		void DeleteItem( ChPosition pos );

		void Empty();
											// Overrides

		virtual void Serialize( ChArchive& ar );

	protected:
		ChParamList		m_itemList;
};

#endif	// !defined( _CHKEYMAP_H )
