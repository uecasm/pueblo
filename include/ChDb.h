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

	Chaco database interface 

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHDB_H )
#define _CHDB_H

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif

#include <ChTypes.h>


/*----------------------------------------------------------------------------
	Constants:
----------------------------------------------------------------------------*/

#define	CHDB_CREATE			0x00000001L
#define CHDB_FAILIFEXISTS   0x00000002L
#define	CHDB_DELETEONCLOSE	0x00000004L
#define CHDB_READ			0x00000008L
#define CHDB_WRITE			0x00000010L
#define CHDB_READWRITE		( CHDB_READ | CHDB_WRITE )


#define CHDB_INSERT			1
#define CHDB_REPLACE		2


/*----------------------------------------------------------------------------
	Types:
----------------------------------------------------------------------------*/ 

class CH_EXPORT_CLASS ChDBKey
{    
	public:
											// Constructors
		ChDBKey( const ChString& strKey );
		ChDBKey( pstr pstrKey );
		ChDBKey( chint32 lKey );  
		ChDBKey( ptr pKey, chint32 lSize );
		ChDBKey( const ChDBKey& Key );
											// Destructor
		virtual ~ChDBKey();
											// operators

		const ChDBKey& operator=( const ChDBKey dbKey);
		const ChDBKey& operator=( const ChString& strKey );
		const ChDBKey& operator=( chint32 lKey );

		const pstr GetKey() const { return m_pstrDBKey; }
		chint32	GetKeySize() const { return m_lKeySize; }

	private:
		pstr		m_pstrDBKey;	
		chint32 	m_lKeySize;
		chint32		m_lAllocSize;
};

class CH_EXPORT_CLASS ChDBData
{
	public:
											// Constructor
		ChDBData( const ChDBData& data );
		ChDBData( const ChString& strData );
		ChDBData( const pstr	 pstrData );
		ChDBData( const ptr	 pData, chint32 lSize );

											// Destructor
		virtual ~ChDBData();
											// Operators

		const ChDBData& operator=( const ChDBData dbData );
		const ChDBData& operator=( const pstr pstrData );
	
											// Methods

		const ptr GetData() const { return m_pData; }
		long GetDataSize() const { return m_lDataSize; }

	private:
		ptr			m_pData;	
		chint32 	m_lDataSize;
		chint32		m_lAllocSize;
};
	

class CH_EXPORT_CLASS ChDataBase	
{
	public :
	// Constructor
	ChDataBase( const char* pstrDBName );
	// destructor
	virtual ~ChDataBase();                    
	// methods
	bool 	  OpenDB( 	chuint32 flOpenFlags = CHDB_READ );
	bool	  IsValidDB()				{ return m_boolValid; }
	ChDBData  GetData( ChDBKey& dbKey ) const;
	bool	  SetData( const ChDBKey& dbKey, const ChDBData& data, 
						chuint32 flOptions = CHDB_INSERT ) const;
	bool 	  Delete( ChDBKey& dbKey ) const;
	ChDBKey   GetFirstKey();
	ChDBKey   GetNextKey( ChDBKey& dbKey ) const; 
	int		  GetLastError();
	
	private :  
	ptr			m_pDataBaseID; 
	pstr    	m_pstrFile;
	chuint32	m_dbFlags;
	bool		m_boolValid;

};
	


/*----------------------------------------------------------------------------
	Functions:
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
	Macros:
----------------------------------------------------------------------------*/

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR    
#endif

// $Log$

#endif	// !defined( _CHDB_H )
