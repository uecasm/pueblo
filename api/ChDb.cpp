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

	Chaco database implementation

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <ChTypes.h>
#include <ChDb.h>                  


#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif


#include <fcntl.h>
#ifdef CH_UNIX
#include <unistd.h>
#include <sys/param.h>
#define _MAX_PATH MAXPATHLEN
#define _O_BINARY 0
#define _O_RDONLY O_RDONLY
#define _O_WRONLY O_WRONLY
#define _O_CREAT O_CREAT
#define _O_EXCL O_EXCL
#define _S_IREAD S_IREAD
#define _S_IWRITE S_IWRITE
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>      
#if defined( CH_MSW) && defined( CH_ARCH_16 )
#include<io.h>                               
#endif

#include "ChNdbm.h"

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	Macros:
----------------------------------------------------------------------------*/

#if defined( CH_MSW )

	#undef	strcpy
	#undef	strcat
	#undef	strlen

	#define strcpy		lstrcpy
	#define strcat		lstrcat
	#define strlen		lstrlen

	#if defined( CH_ARCH_16 )

		#define strchr		_fstrchr
		#define strrchr		_fstrrchr
		#define access		_access

	#endif	// defined( CH_ARCH_16 )

#endif	// defined( CH_MSW )

#ifdef CH_UNIX
	#define _access access
#endif


/*----------------------------------------------------------------------------
	ChDBKey class
----------------------------------------------------------------------------*/

// Constructors

ChDBKey::ChDBKey( const ChString& strKey )
{
#ifdef CH_MSW
	m_lKeySize = strKey.GetLength();
#else
	m_lKeySize = strKey.length();
#endif
	ASSERT( m_lKeySize );

	m_pstrDBKey = new char[m_lKeySize + 1];
	ASSERT( m_pstrDBKey );
	strcpy( m_pstrDBKey, strKey );
	m_lAllocSize = m_lKeySize + 1;
}

ChDBKey::ChDBKey( pstr pstrKey )
{
	m_lKeySize = strlen( pstrKey );
	ASSERT( m_lKeySize );
	m_pstrDBKey = new char[ m_lKeySize + 1];
	ASSERT( m_pstrDBKey );
	strcpy( m_pstrDBKey, pstrKey );
	m_lAllocSize = m_lKeySize + 1;
}

ChDBKey::ChDBKey( chint32 lKey )
{
	m_pstrDBKey = new char[ 20];
	ASSERT( m_pstrDBKey );
	::wsprintf( m_pstrDBKey, "%ld", lKey );
	m_lKeySize = lstrlen( m_pstrDBKey );
	m_lAllocSize = 20;
}

ChDBKey::ChDBKey( ptr pData, chint32 lSize )
{
	if ( lSize )
	{
		m_pstrDBKey = new char[ lSize];
		ASSERT( m_pstrDBKey );
		ChMemCopy( m_pstrDBKey, pData, lSize );
		m_lKeySize = lSize;
		m_lAllocSize = m_lKeySize;
	}
	else
	{
		m_pstrDBKey = NULL;
		m_lAllocSize = m_lKeySize = 0;
			
	}

}

ChDBKey::ChDBKey( const ChDBKey& key )
{
	m_lKeySize = key.m_lKeySize;
	m_lAllocSize = key.m_lKeySize;

	if ( m_lKeySize )
	{

		ASSERT( m_lKeySize );
		m_pstrDBKey = new char[ m_lKeySize];
		ASSERT( m_pstrDBKey );
		ChMemCopy( m_pstrDBKey, key.m_pstrDBKey, m_lKeySize );
	}
	else
	{
		m_pstrDBKey = NULL;
		m_lAllocSize = m_lKeySize = 0;
	}
}


// Destructor
ChDBKey::~ChDBKey()
{
	if ( m_pstrDBKey )
	{
		delete []m_pstrDBKey;
	}
}

/*----------------------------------------------------------------------------
	ChDBKey operators
----------------------------------------------------------------------------*/

const ChDBKey& ChDBKey::operator=( const ChDBKey newKey )
{
	if ( m_pstrDBKey != newKey.m_pstrDBKey )
	{
		if ( newKey.m_lKeySize > m_lAllocSize )
		{
			delete []m_pstrDBKey;
			ASSERT( m_pstrDBKey );
			m_pstrDBKey = new char[ newKey.m_lKeySize];
			m_lAllocSize = newKey.m_lKeySize;
		}
		m_lKeySize = newKey.m_lKeySize;
		ChMemCopy( m_pstrDBKey, newKey.m_pstrDBKey, m_lKeySize );
	}

	return *this;
}

const ChDBKey& ChDBKey::operator=( const ChString& strKey )
{
#ifdef CH_MSW
	chint32 iLength = strKey.GetLength();
#else
	chint32 iLength = strKey.length();
#endif
	if ( iLength > m_lAllocSize )
	{
		delete []m_pstrDBKey;
		m_pstrDBKey = new char[ iLength + 1];
		ASSERT( m_pstrDBKey );
		m_lAllocSize = iLength + 1;

	}
	m_lKeySize = iLength;
	strcpy( m_pstrDBKey, strKey );
	return *this;
}

const ChDBKey& ChDBKey::operator=( chint32 lKey )
{
	if ( 20 > m_lAllocSize )
	{
		delete []m_pstrDBKey;
		m_pstrDBKey = new char[ 20 ];
		m_lAllocSize = 20;
	}
	::wsprintf( m_pstrDBKey, "%ld", lKey );
	m_lKeySize = lstrlen( m_pstrDBKey );
	return *this;
}

/*----------------------------------------------------------------------------
	ChDBData
----------------------------------------------------------------------------*/

// Constructor
ChDBData::ChDBData( const ChString& strData )
{
#ifdef CH_MSW
	m_lAllocSize = m_lDataSize = strData.GetLength();
#else
	m_lAllocSize = m_lDataSize = strData.length();
#endif
	if ( m_lDataSize )
	{
		m_pData = new char[ m_lDataSize + 1];
		m_lAllocSize++;
		ASSERT( m_pData );
		strcpy( (pstr)m_pData, strData );
	}
	else
	{
		m_pData = NULL;
	}
}

ChDBData::ChDBData( const ChDBData& chData )
{
	m_lAllocSize = m_lDataSize = chData.m_lDataSize;
	if ( m_lDataSize )
	{
		m_pData = new char[ m_lDataSize];
		ASSERT( m_pData );
		ChMemCopy( m_pData, chData.m_pData, m_lDataSize );
	}
	else
	{
		m_pData = NULL;
	}
}


ChDBData::ChDBData( const pstr	 pstrData )
{
	m_lAllocSize = m_lDataSize = lstrlen( pstrData );
	if ( m_lDataSize )
	{
		m_pData = new char[ m_lDataSize + 1];
		m_lAllocSize++;
		ASSERT( m_pData );
		strcpy( (pstr)m_pData, pstrData );
	}
	else
	{
		m_pData = NULL;
	}

}

ChDBData::ChDBData( const ptr	 pData, chint32 lSize )
{
	if ( lSize )
	{
		m_pData = new char[ lSize];
		ASSERT( m_pData );
		ChMemCopy( m_pData, pData, lSize );
		m_lAllocSize = m_lDataSize = lSize;
	}
	else
	{
		m_pData = NULL;
		m_lAllocSize = m_lDataSize = 0;
	}
}
//Destructor
ChDBData::~ChDBData()
{
	if ( m_pData )
	{
		delete []m_pData;
	}
}

const ChDBData& ChDBData::operator=( const ChDBData newData )
{
	if ( m_pData != newData.m_pData )
	{
		if ( newData.m_lDataSize > m_lAllocSize )
		{
			delete []m_pData;
			m_pData = new char[ newData.m_lDataSize];
			ASSERT( m_pData );
			m_lAllocSize = newData.m_lDataSize;
		}
		m_lDataSize = newData.m_lDataSize;
		ChMemCopy( m_pData, newData.m_pData, m_lDataSize );
	}

	return *this;
}

const ChDBData& ChDBData::operator=( const pstr	 pstrData )
{
	m_lDataSize = lstrlen( pstrData );
	if ( (m_lDataSize + 1) > m_lAllocSize )
	{
		delete []m_pData;
		m_pData = new char[ m_lDataSize + 1];
		m_lAllocSize = m_lDataSize + 1;
		ASSERT( m_pData );
	}
	strcpy( (pstr)m_pData, pstrData );

	return *this;

}




/*----------------------------------------------------------------------------
	ChDataBase 	  class
----------------------------------------------------------------------------*/

// Constructor

ChDataBase::ChDataBase( const char* pstrDBName )
{
	// Allocate buffer to store the file name

	m_pstrFile = new char[_MAX_PATH];
	ASSERT( m_pstrFile );

	if ( !pstrDBName )
	{
		pstr pstrExt;
		// Get the temp file name
		pstrDBName = m_pstrFile;

		#if defined( CH_MSW ) && defined( CH_ARCH_16 )
		GetTempFileName( GetTempDrive( 0 ),  TEXT( "CH" ), 0, m_pstrFile );
		// remove the extension
		pstrExt = _fstrrchr( pstrDBName, TEXT( '.' ));

		#elif defined( CH_MSW ) && defined( CH_ARCH_32 )
		{
			char strTemp[_MAX_PATH];
			GetTempPath( _MAX_PATH, strTemp );
			GetTempFileName( strTemp,  TEXT( "CH" ), 0, m_pstrFile );
			// remove the extension
			pstrExt = strrchr( (pstr)pstrDBName, TEXT( '.' ));
		}
		#elif defined( CH_UNIX )
		{
			strcpy( pstrDBName, tempnam( 0, "CH" ) );
			pstrExt = 0;
		}
		#endif

		if ( pstrExt )
		{
			*pstrExt = TEXT( '\0' );
		}
		m_boolValid = false;

	}
	else
	{
		m_boolValid = true;

		::strcpy( m_pstrFile, pstrDBName );
			// check if this is a valid database
		char	strDBName[MAX_PATH];
		strcpy(strDBName, m_pstrFile);
		strcat(strDBName, ".pag");

		#if defined( CH_MSW ) && defined( CH_ARCH_32 )
		if (  ::GetFileAttributes( strDBName ) == 0xFFFFFFFF )
		{
			m_boolValid = false;
		}
		#else
		if (_access( strDBName, 0 ))
		{
			m_boolValid = false;
		}
		#endif

		if ( m_boolValid )
		{
			strcpy(strDBName, m_pstrFile);
			strcat(strDBName, ".dir");

			#if defined( CH_MSW ) && defined( CH_ARCH_32 )
			if (  ::GetFileAttributes( strDBName ) == 0xFFFFFFFF )
			{
				m_boolValid = false;
			}
			#else
			if (access( strDBName, 0 ))
			{
				m_boolValid = false;
			}
			#endif
		}
	}



	m_pDataBaseID = NULL;
	m_dbFlags	  = 0;
}

	// destructor
ChDataBase::~ChDataBase()
{
	if ( m_pDataBaseID )
	{
		dbm_close( (pDBM)m_pDataBaseID );
	}

	if ( m_pDataBaseID && ( m_dbFlags & CHDB_DELETEONCLOSE ))
	{// Delete the .dir and .pag files
		char		astrFile[_MAX_PATH];
		#if defined( CH_ARCH_16 )
		strcpy(astrFile, m_pstrFile);
		strcat(astrFile, ".dir");
		// delete the file
		remove( astrFile );

		strcpy(astrFile, m_pstrFile);
		strcat(astrFile, ".pag");
		// delete the file
		remove( astrFile );

		#else
		strcpy(astrFile, m_pstrFile);
		strcat(astrFile, ".dir");
		// delete the file
		DeleteFile( astrFile );

		strcpy(astrFile, m_pstrFile);
		strcat(astrFile, ".pag");
		// delete the file
		DeleteFile( astrFile );
		#endif

	}

	if ( m_pstrFile )
	{
		delete []m_pstrFile;
	}

}


/*----------------------------------------------------------------------------
	ChDataBase 	  public methods
----------------------------------------------------------------------------*/

bool ChDataBase::OpenDB( chuint32 flOpenFlags)
{
	// Allocate buffer to store the file name

	// save the user flags
	m_dbFlags  = flOpenFlags;

	int iFlags = _O_BINARY;

	iFlags |= (m_dbFlags & CHDB_READ ) ? _O_RDONLY : 0;
	iFlags |= (m_dbFlags & CHDB_WRITE ) ? _O_WRONLY : 0;
	iFlags |= (m_dbFlags & CHDB_CREATE ) ? _O_CREAT : 0;
	iFlags |= (m_dbFlags & CHDB_FAILIFEXISTS ) ? _O_EXCL : 0;

	m_pDataBaseID = (ptr)dbm_open( m_pstrFile, iFlags, _S_IREAD | _S_IWRITE );

	return( m_pDataBaseID != NULL );

}


ChDBData ChDataBase::GetData( ChDBKey& dbKey ) const
{
	datum	 data;
	datum	 key;
	key.dptr = dbKey.GetKey();
	key.dsize= dbKey.GetKeySize();


	data = dbm_fetch( (pDBM)m_pDataBaseID, key);

	// Make a new data
	ChDBData dbData( data.dptr, data.dsize);

	return (dbData);
}


bool ChDataBase::SetData( const ChDBKey& dbKey, const ChDBData& data,
							chuint32 flOptions ) const
{
	chint32	iFlags = 0;
	datum	key;
	datum	dbdata;

	iFlags |= (flOptions & CHDB_INSERT) ? DBM_INSERT : 0;
	iFlags |= (flOptions & CHDB_REPLACE) ? DBM_REPLACE : 0;

	key.dptr = dbKey.GetKey();
	key.dsize= dbKey.GetKeySize();

	dbdata.dptr = data.GetData();
	dbdata.dsize= data.GetDataSize();

	chint32 iRet = dbm_store( (pDBM)m_pDataBaseID, key,
							dbdata, iFlags );

	return (iRet == 0);
}

bool ChDataBase::Delete( ChDBKey& dbKey ) const
{
	datum	 key;
	key.dptr = dbKey.GetKey();
	key.dsize= dbKey.GetKeySize();

	chint32 iRet = dbm_delete( (pDBM)m_pDataBaseID, key );

	return (iRet == 0);
}

ChDBKey ChDataBase::GetFirstKey()
{
	datum  key = dbm_firstkey( (pDBM)m_pDataBaseID );

	ChDBKey dbKey( key.dptr, key.dsize );

	return dbKey;
}

ChDBKey ChDataBase::GetNextKey( ChDBKey& dbKey ) const
{

	datum  key = dbm_nextkey( (pDBM)m_pDataBaseID );

	ChDBKey dbNextKey( key.dptr, key.dsize );

	return dbNextKey;
}

int ChDataBase::GetLastError()
{
	return (int)dbm_error( (pDBM)m_pDataBaseID );
}

// $Log$
