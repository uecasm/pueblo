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

	This file consists of implementation of the ChModule class.

----------------------------------------------------------------------------*/

#include <string.h>

#include "headers.h"

#ifdef CH_UNIX
#include <stdlib.h>
#include <ChTypes.h>
#include <ChConst.h>
#include <ChUtil.h>

#define ERROR_SUCCESS 		1
#define ERROR_NO_DATA		2

#define HKEY_CLASSES_ROOT	(void *)2
#define REG_SZ				3
#endif // CH_UNIX

#include <ChUtil.h>
#include <ChReg.h>

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/


#define SOFTWARE_KEY		"Software"


/*----------------------------------------------------------------------------
	Static variables
----------------------------------------------------------------------------*/

#ifdef CH_MSW
static ChString strRegistryFile;
#endif

const char *strTrue = "true";
const char *strFalse = "false";

// Unix versions of the MSW Registry functions:
#ifdef CH_UNIX

ChDataBase* ChRegistry::m_pRegDB = 0;
chuint32	ChRegistry::m_luRefCount = 0;

chint32 RegQueryValue( HKEY hKey, const ChString& strLabel, char* pstrBuffer,
					   chint32* pLen )
{
	cerr << "RegQueryValue( " << hKey << ", " << strLabel << " )" << endl;

	ChString strDbKey = (*(ChString*)hKey) + "/" + strLabel;
	ChDBKey	dbKey( strDbKey );

#if 0
	if (strLabel == CH_MAX_CONNECTIONS) {
		strcpy( pstrBuffer, "5" );
		*pLen = 1;
	} else {
		cerr << "RegQueryValue( " << strLabel << ")" << endl;
		strcpy(pstrBuffer, "");
		*pLen = 0;
	}
#endif

	ChDBData dbData = ChRegistry::m_pRegDB->GetData( dbKey );
	if (dbData.GetDataSize()) {
		// If their buffer is bigger than the data, tell them how
		// much we're really copying.
		if (*pLen > dbData.GetDataSize())
			*pLen = dbData.GetDataSize();

		memcpy( pstrBuffer, dbData.GetData(), *pLen );
		return ERROR_SUCCESS;
	} else {
		return ERROR_NO_DATA;
	}
}

// luFlags might be REG_SZ
chint32 RegSetValue( HKEY hKey, const ChString& strLabel, chuint32 luFlags,
					 char* pstrBuffer, chuint32 lLen )
{
	cerr << "RegSetValue( " << hKey << ", " << strLabel << ", "
		 << luFlags << ", " << pstrBuffer << ", " << lLen << " )" << endl;

	ChString strDbKey = (*(ChString*)hKey) + "/" + strLabel;
	ChDBKey	dbKey( strDbKey );
	ChDBData dbData( pstrBuffer, lLen );

	if (ChRegistry::m_pRegDB->SetData( dbKey, dbData, CHDB_REPLACE ))
		return ERROR_SUCCESS;
	else
		return ERROR_NO_DATA;
}

// hKey might be HKEY_CLASSES_ROOT
// hSoftwareKey might be SOFTWARE_KEY
chint32 RegCreateKey( HKEY hKey, char *pstrSoftwareKey, HKEY* pNewKey )
{
	ChString *pStr;

	cerr << "RegCreateKey( " << hKey << ", " << pstrSoftwareKey << " )" << endl;
	if (hKey == HKEY_CLASSES_ROOT) {
		pStr = new ChString(pstrSoftwareKey);
	} else {
		pStr = new ChString(*(ChString*)hKey);
		*pStr += "/";
		*pStr += pstrSoftwareKey;
	}
	*pNewKey = (HKEY)pStr;
	return ERROR_SUCCESS;
}

chint32 RegCloseKey( HKEY hKey )
{
	cerr << "RegCloseKey( " << hKey << " )" << endl;
	delete (ChString *)hKey;
	return ERROR_SUCCESS;
}

void ChRegistry::OpenDb(void)
{
	m_luRefCount++;

	if (m_pRegDB == NULL)
	{
		m_pRegDB = new ChDataBase( ChUtil::GetPuebloDir() + "/" +
									CH_REG_DB_NAME );
		if (m_pRegDB->IsValidDB()) {
									// open the database
			m_pRegDB->OpenDB( CHDB_READWRITE );
		} else {
			m_pRegDB->OpenDB( CHDB_CREATE | CHDB_READWRITE );
		}
	}
}
#endif // CH_UNIX


/*----------------------------------------------------------------------------
	ChRegistry class
----------------------------------------------------------------------------*/

ChRegistry::ChRegistry( const ChString& strCompanyName,
						const ChString& strProduct, const ChString& strGroup,
						const ChString& strVersion ) :
				m_strCompanyName( strCompanyName ),
				m_strProduct( strProduct ),
				m_strGroup( strGroup ),
				m_strVersion( strVersion )
{
	Construct( strCompanyName, strProduct, strGroup, strVersion );

	#if defined( CH_UNIX )
	{
		OpenDb();
	}
	#endif	// defined( CH_UNIX )
}


ChRegistry::ChRegistry( const ChString& strGroup, const ChString& strVersion ) :
				m_strCompanyName( CH_COMPANY_NAME ),
				m_strProduct( CH_PRODUCT_NAME ),
				m_strGroup( strGroup ),
				m_strVersion( strVersion )
{
	Construct( CH_COMPANY_NAME, CH_PRODUCT_NAME, strGroup, strVersion );

	#if defined( CH_UNIX )
	{
		OpenDb();
	}
	#endif	// defined( CH_UNIX )
}


ChRegistry::~ChRegistry()
{
	#if !defined( CH_ARCH_16 )
	if ( m_hMasterKey )
	{											// Close the master key
		RegCloseKey( m_hMasterKey );
	}
	#endif

	#if defined( CH_UNIX )
	{
		m_luRefCount--;
		if (m_luRefCount == 0)
		{
			delete m_pRegDB;
			m_pRegDB = 0;
		}
	}
	#endif	// defined( CH_UNIX )
}


bool ChRegistry::GetHelpPath( ChString& strHelpFiles )
{
	bool	boolFound = false;

	#if defined( CH_MSW )
	{
		int iType = ChUtil::GetSystemType();

		if (iType == CH_SYS_WIN32S || iType == CH_SYS_WIN3X)
		{
			TRACE( "ChRegistry::GetHelpPath() : Not implemented for 16-bit\n" );
			ASSERT( false );
		}
		else
		{									// all true 32 bit OS
			HKEY		hKey;
			chint32		lError;
			ChString		strKey( SOFTWARE_KEY "\\" );

			strRegistryFile.Empty();

			ASSERT( m_strCompanyName.GetLength() );
			ASSERT( m_strProduct.GetLength() );
			ASSERT( m_strVersion.GetLength() &&
					(m_strVersion != CH_VERSION_DEFAULT) );

											// Create the key

			strKey += m_strCompanyName + "\\" + m_strProduct + "\\" +
						m_strVersion + "\\";
			strKey += CH_GROUP_PATH;
			lError = RegCreateKey( HKEY_LOCAL_MACHINE, strKey, &hKey );
			if (lError == ERROR_SUCCESS)
			{
				DWORD	dwType;
				char	buffer[MAX_PATH];
				DWORD	dwSize = MAX_PATH;
				LONG	lResult;

				lResult = RegQueryValueEx( hKey, CH_KEY_HELPFILES, 0,
											&dwType, (LPBYTE)buffer, &dwSize );

				if ((ERROR_SUCCESS == lResult) && (REG_SZ == dwType))
				{
					strHelpFiles = buffer;
					boolFound = true;
				}
				else
				{
					if (ERROR_SUCCESS != lResult)
					{
						MessageBeep( MB_OK );
					}
				}
			}
		}
	}
	#else	// defined( CH_MSW )
	{
		cerr << "ChRegistry::GetHelpPath()" << endl;
	}
	#endif	// defined( CH_MSW )

	return boolFound;
}


bool ChRegistry::Erase( const ChString& strLabel )
{
	chint32		lRetVal;

	#if defined( _WIN32 )
	{
		if ( !strRegistryFile.IsEmpty() )
		{
			WritePrivateProfileString( m_strMasterKey, strLabel, NULL, strRegistryFile );
		}
		else
		{
			lRetVal = RegDeleteValue( m_hMasterKey, (char*)(const char*)strLabel );
		}
	}
	#elif defined( CH_UNIX )
	{
		TRACE( "Need to erase an item from the registry here.\n" );
	}
	#elif defined( CH_MSW) && defined( CH_ARCH_16 )
	{
		#pragma message( "Need to erase an item from the registry here." );
	}
	#endif

	return( lRetVal == ERROR_SUCCESS );
}


bool ChRegistry::WriteBool( const ChString& strLabel, BOOL boolVal )
{
#ifdef CH_MSW
	ChString	strVal( boolVal ? strTrue : strFalse );

	return Write( strLabel, strVal );
#else
	return Write( strLabel, boolVal ? strTrue : strFalse );
#endif
}


bool ChRegistry::Write( const ChString& strLabel, chint32 lVal )
{
	chint32	lRetVal;

	#if defined( _WIN32 )
	{
		if ( !strRegistryFile.IsEmpty() )
		{
			char strVal[50];
			wsprintf( strVal, "%ld", lVal );

			WritePrivateProfileString( m_strMasterKey, strLabel, strVal, strRegistryFile );
			lRetVal = ERROR_SUCCESS;
		}
		else
		{
			lRetVal = RegSetValueEx( m_hMasterKey, strLabel, 0, REG_DWORD, (chuint8 *)&lVal,
									sizeof( DWORD ) );
		}
	}
	#elif defined( CH_UNIX )	// defined( _WIN32 )
	{
		char	strBuffer[20];

		ltoa( lVal, strBuffer, 10 );


		lRetVal = RegSetValue( m_hMasterKey, strLabel, REG_SZ, strBuffer,
								strlen( strBuffer ) );
	}
	#elif defined( CH_MSW) && defined( CH_ARCH_16 )
	{
		AfxGetApp()->WriteProfileInt( m_strMasterKey, strLabel, (int)lVal );
		lRetVal = ERROR_SUCCESS;
	}
	#endif

	return( lRetVal == ERROR_SUCCESS );
}


bool ChRegistry::Write( const ChString& strLabel, const ChString& strVal )
{
	chint32	lRetVal;

	#if defined( _WIN32 )
	{
		if ( !strRegistryFile.IsEmpty() )
		{
			WritePrivateProfileString( m_strMasterKey, strLabel,
											strVal, strRegistryFile );
			lRetVal = ERROR_SUCCESS;
		}
		else
		{
			lRetVal = RegSetValueEx( m_hMasterKey, strLabel, 0, REG_SZ,
									(const chuint8 *)(const char *)strVal,
									strVal.GetLength() + 1 );
		}
	}
	#elif defined( CH_UNIX )	// defined( _WIN32 )
	{
		lRetVal = RegSetValue( m_hMasterKey, strLabel, REG_SZ, strVal,
								strVal.GetLength() );
	}
	#elif defined( CH_MSW) && defined( CH_ARCH_16 )  // defined( CH_UNIX )
	{
		AfxGetApp()->WriteProfileString( m_strMasterKey, strLabel, strVal );
		lRetVal = ERROR_SUCCESS;
	}
	#endif

	return( ERROR_SUCCESS == lRetVal );
}


bool ChRegistry::Read( const ChString& strLabel, chint32& lVal,
							chint32 lDefault )
{
	chint32	lRetVal;

	#if defined( _WIN32 )
	{


		if ( !strRegistryFile.IsEmpty() )
		{
			lVal = (UINT)GetPrivateProfileInt( m_strMasterKey, strLabel, (int)lDefault, strRegistryFile );
			lRetVal = ERROR_SUCCESS;
		}
		else
		{

			DWORD	dwType;
			DWORD	dwSize = sizeof( chint32 );
			char	strName[80];

			ASSERT( strLabel.GetLength() < sizeof( strName ) );
			strcpy( strName, strLabel );
			lRetVal = RegQueryValueEx( m_hMasterKey, strName, 0,
										&dwType, (chuint8 *)&lVal, &dwSize );

			if ((ERROR_SUCCESS != lRetVal) || (REG_DWORD != dwType) ||
				(dwSize != sizeof( DWORD )))
			{
				lVal = lDefault;
			}
		}
	}
	#elif defined( CH_UNIX )	// defined( _WIN32 )
	{
		char	strBuffer[80];
		chint32	lLen = sizeof( strBuffer );

		lRetVal = RegQueryValue( m_hMasterKey, strLabel, strBuffer, &lLen );
		if (ERROR_SUCCESS == lRetVal)
		{
			lVal = atol( strBuffer );
		}
		else
		{
			lVal = lDefault;
		}
	}
	#elif defined( CH_MSW) && defined( CH_ARCH_16 )  // defined( CH_UNIX )
	{
		lVal = (UINT)AfxGetApp()->GetProfileInt( m_strMasterKey, strLabel, (int)lDefault );
		lRetVal = ERROR_SUCCESS;
	}
	#endif

	return( ERROR_SUCCESS == lRetVal );
}


bool ChRegistry::Read( const ChString& strLabel, chint16& sVal,
							chint16 sDefault )
{
	bool		boolSuccess;
	chint32		lVal;

	boolSuccess = Read( strLabel, lVal, (chint32)sDefault );
	sVal = (chint16)lVal;

	return boolSuccess;
}


bool ChRegistry::ReadBool( const ChString& strLabel, bool& boolVal,
							bool boolDefault )
{
	bool		boolSuccess;
	ChString*		pstrDefault;
	ChString		strResult;

	ChString stringTrue(strTrue);
	ChString stringFalse(strFalse);
	pstrDefault = boolDefault ? &stringTrue : &stringFalse;

	if (boolSuccess = Read( strLabel, strResult, *pstrDefault ))
	{
		if (strResult == strTrue)
		{
			boolVal = true;
		}
		else if (strResult == strFalse)
		{
			boolVal = false;
		}
		else
		{
			boolVal = boolDefault;
			boolSuccess = false;
		}
	}
	else
	{
		boolVal = boolDefault;
	}

	return boolSuccess;
}


bool ChRegistry::Read( const ChString& strLabel, ChString& strVal,
							const ChString& strDefault )
{
	chint32		lRetVal = ERROR_SUCCESS;

	#if !defined( CH_ARCH_16 )
	ChString		strBuffer;
	chint32		lLen;

	char		strName[256];
	#endif // !defined( CH_ARCH_16 )

	#if defined( _WIN32 )
	{
		if ( !strRegistryFile.IsEmpty() )
		{
			char * pstrVal = strVal.GetBuffer( MAX_PATH );
			lRetVal = GetPrivateProfileString( m_strMasterKey, strLabel, strDefault,
										pstrVal, MAX_PATH, strRegistryFile );
			strVal.ReleaseBuffer();

			if ( lRetVal == 0 )
			{
				strVal = strDefault;
			}

			return( true );
		}
		else
		{
			DWORD	dwType;
			DWORD	dwSize = 0;

			ASSERT( strLabel.GetLength() < sizeof( strName ) );
			strcpy( strName, strLabel );
			lRetVal = RegQueryValueEx( m_hMasterKey, strName, 0, &dwType,
										0, &dwSize );
			lLen = dwSize + 2;

			if (dwType != REG_SZ)
			{
				lRetVal = ERROR_NO_DATA;		// Any error code would do
			}
		}
	}
	#elif defined( CH_UNIX )	// defined( _WIN32 )
	{
		lLen = sizeof( strName );
		lRetVal = RegQueryValue( m_hMasterKey, strLabel, strName, &lLen );
	}
	#elif defined( CH_MSW) && defined( CH_ARCH_16 )  // defined( CH_UNIX )
	{
		strVal = AfxGetApp()->GetProfileString( m_strMasterKey, strLabel, strDefault );
		lRetVal = ERROR_SUCCESS;
	}
	#endif

	#if !defined( CH_ARCH_16 )

	if (lRetVal == ERROR_SUCCESS)
	{
		#ifdef CH_UNIX
			char* pBuffer;

			pBuffer = new char [lLen + 1];
			RegQueryValue( m_hMasterKey, strLabel, pBuffer, &lLen );
			strBuffer = pBuffer;
			delete pBuffer;
		#else

			if ( strRegistryFile.IsEmpty() )
			{
				char	*pBuffer = strBuffer.GetBuffer( lLen );

				#if defined( CH_ARCH_32 )
				{
					DWORD	dwSize = lLen;

					lRetVal = RegQueryValueEx( m_hMasterKey, strName, 0, 0,
													(chuint8 *)pBuffer, &dwSize );
				}
				#else	// defined( _WIN32 )
				{
					lRetVal = RegQueryValue( m_hMasterKey, strLabel, pBuffer, &lLen );
				}
				#endif	// defined( _WIN32 )

				strBuffer.ReleaseBuffer();
			}

		#endif
	}

	if (lRetVal == ERROR_SUCCESS)
	{
		strVal = strBuffer;
	}
	else
	{
		strVal = strDefault;
	}

	#endif //   !defined( CH_ARCH_16 )

	return( ERROR_SUCCESS == lRetVal );
}


/*----------------------------------------------------------------------------
	ChRegistry protected methods
----------------------------------------------------------------------------*/

void ChRegistry::Construct( const ChString& strCompanyName,
							const ChString& strProduct, const ChString& strGroup,
							const ChString& strVersion )
{
	#if defined( CH_MSW )
	{
		int iType = ChUtil::GetSystemType();

		if (iType == CH_SYS_WIN32S || iType == CH_SYS_WIN3X)
		{
											// main section name

			m_strMasterKey =  strCompanyName + TEXT( "//" ) +
								strProduct + TEXT( "//" ) + strGroup;
			m_hMasterKey   = 0;

			if ( strRegistryFile.IsEmpty() )
			{
				char * pstrBuffer = strRegistryFile.GetBuffer( MAX_PATH );
				GetWindowsDirectory( pstrBuffer, MAX_PATH );
				strRegistryFile.ReleaseBuffer();

				#if defined(CH_VRML_VIEWER)	|| defined(CH_VRML_PLUGIN )
				strRegistryFile += TEXT( "\\VRScout.ini" );
				#else
				strRegistryFile += TEXT( "\\Pueblo.ini" );
				#endif
			}


		}
		else
		{									// all true 32 bit OS
			HKEY		hTempKey;
			chint32		lError;
			ChString		strKey( SOFTWARE_KEY "\\" );

			strRegistryFile.Empty();

			ASSERT( strCompanyName.GetLength() );
			ASSERT( strProduct.GetLength() );
			ASSERT( strGroup.GetLength() );
											// Create the master key

			strKey += m_strCompanyName + "\\" + strProduct + "\\" + strGroup;
			lError = RegCreateKey( HKEY_CURRENT_USER, strKey, &hTempKey );
			if (lError != ERROR_SUCCESS)
			{
				m_hMasterKey = 0;
			}
			else
			{
				m_hMasterKey = hTempKey;
			}
		}
	}
	#else	// defined( CH_MSW )
	{
		cerr << "ChRegistry::Construct()" << endl;
	}
	#endif	// defined( CH_MSW )
}


// Local Variables: ***
// tab-width:4 ***
// End: ***
