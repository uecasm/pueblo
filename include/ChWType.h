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

#if !defined( _CHWTYPE_H )
#define _CHWTYPE_H

#include <ChStrmbl.h>


/*----------------------------------------------------------------------------
	Types:
----------------------------------------------------------------------------*/

typedef enum { undefinedLogin = -1, variableLogin = 0, connectLogin,
				unamePwLogin } ChLoginType;

typedef enum { endTypeList = -1, undefinedType = 0, Aber, Circle, DGD, Diku,
				LPMud, Merc, Moo, Muck, Muse, Mux, Talker, TinyMush,
				otherType } WorldType;

typedef struct
{
	WorldType	type;
	char*		pstrName;
	ChLoginType	login;

} WorldTypeName;


/*----------------------------------------------------------------------------
	ChWorldType class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChWorldType : public ChStreamable
{
	public:
		enum { constVersion = 1 };

	public:
		ChWorldType() : m_type( otherType ), m_loginType( variableLogin ) {}
		ChWorldType( const ChString& strType );
		ChWorldType( const ChWorldType& type );
		ChWorldType( WorldType type );
		virtual ~ChWorldType() {}

		inline operator WorldType() const { return m_type; }
		inline ChLoginType GetLoginType() const { return m_loginType; }
		inline bool IsValidType() const
				{
					return (m_type > 0);
				}
		inline void Set( WorldType type )
				{
					m_type = type;
				}

		#if defined( CH_MSW )

		static void FillTypeList( CComboBox* pCombo );

		#endif	// defined( CH_MSW )

		virtual void Serialize( ChArchive& ar );

		ChString GetName() const;
		void Set( const ChString& strType );

	private:
		static WorldTypeName	m_typeNames[];
		WorldType				m_type;
		ChLoginType				m_loginType;
};


#endif	// !defined( _CHWTYPE_H )

// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
