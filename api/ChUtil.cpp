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

	This file contains the utility functions.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#if !defined(CH_PUEBLO_PLUGIN)
#include "resource.h"
#else
#include "vwrres.h"
#endif

#if defined( CH_MSW )

	#if defined( CH_ARCH_16 )
		#include <sys\types.h>  
		#include <sys\stat.h>  
		#include <fcntl.h>
	#endif	

#elif defined( CH_UNIX )

	#include <stdlib.h>
	#include <unistd.h>
	#include <sys/stat.h>
	#include <ChTypes.h>

#endif

#include <ChUtil.h>
#include <ChClInfo.h>

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define ENCRYPT_ESCAPE			'\\'
#define ENCRYPT_HEX_CODE		'x'
#define ENCRYPT_STYLE_SIMPLE	'\x01'

/*----------------------------------------------------------------------------
	Types
----------------------------------------------------------------------------*/

 class ChDllLoadInfo
{
	public :
		ChDllLoadInfo( const ChString& strModule,  HMODULE hLibrary ) 	:
							 m_strModule( strModule ),
							 m_hLibrary( hLibrary )
							 {
							 }
 		const ChString& GetModuleName()		{ return m_strModule; }
 		HMODULE GetLibrary()		{ return m_hLibrary; }

	private :
		ChString 	 	m_strModule; 
		HMODULE	m_hLibrary;

							
};



class ChDllLoadManager
{
	public :
		ChDllLoadManager()		{}
		~ChDllLoadManager();
		// Methods
		ChParamList& GetLoadList()  		{ return m_loadLst; }
	private :
		ChParamList 	m_loadLst;
	
};

/*----------------------------------------------------------------------------
	ChUtil class static values
----------------------------------------------------------------------------*/

ChString				ChUtil::m_strAppDirectory;
bool				ChUtil::m_directorySet;
int					ChUtil::m_iSysType = CH_SYS_UNKNOWN;
chflag32			ChUtil::m_flSysProps = (chflag32)-1;
ChTmpFileManager 	ChUtil::m_tmpFileMgr;


static ChDllLoadManager 	dllLoadMgr;

#if defined( CH_UNIX )

ChString			ChUtil::m_strPuebloDirectory;

#endif

const char*	ChUtil::m_pstrEncryptKey =
									"Spaulding Grey is the official parrot "
									"of Andromedia Incorporated";


/*----------------------------------------------------------------------------
	ChUtil class
----------------------------------------------------------------------------*/

bool ChUtil::Load( chparam resources, chint32 lKey, ChString& strData )
{
	bool	boolLoaded;

	#if defined( CH_MSW )
	{
		HINSTANCE	hModule = (HINSTANCE)resources;
		HINSTANCE	hInstOld = AfxGetResourceHandle();

		AfxSetResourceHandle( hModule );
		boolLoaded = (strData.LoadString( (int)lKey ) != FALSE);
											// Restore the old resource chain
		AfxSetResourceHandle( hInstOld );
	}
	#elif defined( CH_UNIX )
	{
		int					i;
		string_resource*	pRes = (string_resource*)resources;

		boolLoaded = false;

		for (i = 0; pRes[i].resource_val; i++)
		{
			if (pRes[i].resource_val == lKey)
			{
				strData = pRes[i].resource_string;
				boolLoaded = true;
				break;
			}
		}
	}
	#else
	{
		#error "ChUtil::Load function not defined for this OS"
	}
	#endif

	return boolLoaded;
}


bool ChUtil::FileExists( const char* pstrFilename )
{
	#if defined( CH_MSW )
	{
		#if defined( WIN32 )
		{
			if (::GetFileAttributes( pstrFilename ) == 0xFFFFFFFF)
			{
				return false;
			}
		}
		#else	// defined( WIN32 )
		{
			struct _stat		temp_stat;

			if (_stat( pstrFilename, &temp_stat ) == -1)
			{
				return false;
			}
		}
		#endif	// defined( WIN32 )
	}
	#elif defined( CH_UNIX )
	{
		struct stat		temp_stat;

		if (stat( pstrFilename, &temp_stat ) == -1)
		{
			return false;
		}
	}
	#else
	{
		#error "ChUtil::FileExists function not defined for this OS"
	}
	#endif

	return true;
}

bool ChUtil::GetTempFileName( ChString& strTmpFile, const char* pstrTempPath, 
								const char* pstrPrefix, const char* pstrExtn )
{
	if (!pstrPrefix)
	{
		pstrPrefix = "Ch";
	}	

	#if defined( CH_MSW )
	char   strPath[MAX_PATH];

	strTmpFile.Empty();

		#if defined(CH_ARCH_16 )
			GetTempFileName( GetTempDrive( 0 ), pstrPrefix, 0, strPath );

			if ( pstrTempPath )
			{
				::DeleteFile( strPath );

				char *pstrFileName = _fstrrchr( strPath, PATH_SEPARATOR_CHAR );
				ASSERT( pstrFileName );

				strTmpFile = pstrTempPath;

				if ( pstrExtn )
				{
					::DeleteFile( strPath );
					char *pstrTmp = _fstrrchr( pstrFileName, TEXT( '.' ) );

					if ( pstrTmp  )
					{
						*pstrTmp = 0;
					}
				}
				strTmpFile += pstrFileName;
			}

		#else
			char strTmpPath[MAX_PATH];

			if ( pstrTempPath == 0 )
			{
			 	GetTempPath( sizeof( strTmpPath ), strTmpPath );
				pstrTempPath = strTmpPath;
			}
			
			::GetTempFileName( pstrTempPath, pstrPrefix, 0, strPath );
	
			if ( pstrExtn )
			{
				::DeleteFile( strPath );
				char *pstrTmp = strrchr( strPath, TEXT( '.' ) );
				if ( pstrTmp )
				{
					*pstrTmp = 0;
				}
			}

			strTmpFile = strPath;
		#endif

		if ( pstrExtn )
		{

			if ( *pstrExtn != TEXT('.') )
			{
				strTmpFile += TEXT('.');	
			}

			strTmpFile += pstrExtn;
			if ( ChUtil::FileExists( strTmpFile ) )
			{
				strTmpFile = strPath;
				strTmpFile += TEXT( ".tmp" );
			}

		}

	#else
	{
		char*			pstrTemp;
		char*			pstrTempIn;

		pstrTempIn = new char[MAX_PATH];
		sprintf( pstrTempIn, "%s/%s/ChXXXXXX",
					(const char *)m_strPuebloDirectory, CACHE_DIR );
		pstrTemp = mktemp( pstrTempIn );
		ASSERT( pstrTemp );
		strTmpFile = pstrTemp;

		delete []pstrTempIn;  

		if ( pstrExtn )
		{
			::DeleteFile( (const char*)strTmpFile );

			if ( *pstrExtn != '.' )
			{
				strTmpFile += '.';	
			}
			strTmpFile += pstrExtn;
		}
	}
	#endif

	return true;

}


void ChUtil::AddFileToTempList( const ChString& strFile )
{
	m_tmpFileMgr.Add( strFile );
}

void ChUtil::EmptyTempFileList()
{
	m_tmpFileMgr.Empty( );
}


bool ChUtil::SetFileToCurrentTime( const char* pstrFilename )
{
   	bool boolResult = false;
	#if defined( CH_MSW )
	#if defined( CH_ARCH_32 )
    FILETIME ft;
    SYSTEMTIME st;
 	HANDLE hf;

	hf = ::CreateFile( pstrFilename, GENERIC_READ | GENERIC_WRITE,
					 FILE_SHARE_READ,
					 NULL,
					 OPEN_EXISTING,
					 0,
					 0 );
	if ( hf != INVALID_HANDLE_VALUE )
	{

	    ::GetSystemTime(&st);              // gets current time
	    ::SystemTimeToFileTime(&st, &ft);  // converts to file time format
	    boolResult = (::SetFileTime(hf,    // sets last-write time for file
	        (LPFILETIME) NULL, (LPFILETIME) NULL, &ft) != FALSE);
		::CloseHandle( hf );
	}

    return boolResult;
	#else                    
	{   
		int handle;
		if ( _dos_open( pstrFilename, _O_RDWR, &handle ) == 0 )
		{
			struct _dosdate_t ddate;
			struct _dostime_t dtime;   
			_dos_getdate( &ddate );  
	 		_dos_gettime( &dtime );   
	 		unsigned date, time, month, year, hour, min;
	 		
	 		year = ddate.year; 
	 		month = ddate.month; 
	 		min  = dtime.minute;
	 		hour = dtime.hour;
	
	 		date = ( (ddate.day & 0x0F ) | 
	 						(month << 4 ) | (year << 8 ) );
	 		time = ( (dtime.second & 0x0F ) | 
	 						(min << 4 ) | (hour << 10 ) );
	 		// set the time
	 		_dos_setftime( handle,date, time );
	 		_dos_close( handle );       
	 		boolResult = true;
	 	}  
	 	return boolResult;
	 }   
	#endif
	#else
		cerr << "XXX" << __FILE__ << ":" << __LINE__ << endl;	
	#endif

}


void * ChUtil::OpenDirectory( const char* pstrPath, const char* pstrFilter, chuint32 flMode )
{
	pChDirInfo			pDir;
                                       
	pDir = new ChDirInfo;

	if ( pDir == 0 )
	{
		return 0;
	}

	#if defined( CH_MSW )

	pDir->boolFirst = true;
	pDir->flMode = flMode;  
	lstrcpy( pDir->astrPath,  pstrPath );  
	
	if ( pstrFilter )
	{
		if ( pDir->astrPath[lstrlen(pDir->astrPath) - 1] != TEXT( '\\' ) ) 
		{
			lstrcat( pDir->astrPath, TEXT( "\\" ));
			lstrcat( pDir->astrPath, pstrFilter );
		}
	}
		#if !defined( CH_ARCH_32 )    
    
		pDir->hFindFirst = _dos_findfirst( pDir->astrPath, (int)flMode,  
										&pDir->findFile );
	
		if (pDir->hFindFirst )
		{ 
			delete pDir;
			return ( 0 );
		}   
		#else
		{

			pDir->hFindFirst 	= FindFirstFile( pDir->astrPath, &pDir->findFile);
			if (pDir->hFindFirst == INVALID_HANDLE_VALUE)
			{ 
				delete pDir;
				return ( 0 );
			}
	
		}
		#endif
	#else
		cerr << "XXX" << __FILE__ << ":" << __LINE__ << endl;	
	#endif

	return( pDir );
}

#if defined( CH_MSW )
#if defined( CH_ARCH_16 )
bool ChUtil::ReadDirectory( void * pDirHandle, pChFileAttrs pAttrs, chuint flRequest )
{
    bool      	boolSuccess = false;
    pChDirInfo 	pfData = (pChDirInfo)pDirHandle;

	if (!pfData->boolFirst)
	{
		if ( !pfData->hFindFirst )
		{
			boolSuccess = 0 == _dos_findnext( &pfData->findFile);
		}
	
	}
	else
	{
		pfData->boolFirst = false;
	}   

	if ( boolSuccess )
	{	
	
		if ( pfData->findFile.attrib & _A_SUBDIR )
		{
			pAttrs->uFileType = ChUtil::typeDir;
		}
		else
		{
			pAttrs->uFileType = ChUtil::typeFile;
		}
	
	
	
												/* File length attributes */
	
		if ( flRequest & reqSize )
		{
			// size of the file
			pAttrs->luSize =  pfData->findFile.size;
		}

		if ( flRequest & reqPath || flRequest & reqTime )
		{
			// size of the file

	        lstrcpy( pAttrs->astrName,  pfData->astrPath );
			int suDirPathLen = lstrlen( pAttrs->astrName ) - 1;
			while ( suDirPathLen && pAttrs->astrName[suDirPathLen] != TEXT( '\\' ))
			{
				--suDirPathLen;
			}
			lstrcpy( &pAttrs->astrName[suDirPathLen + 1],  pfData->findFile.name );
		}

		// get the time
		if ( flRequest & reqTime )
		{
			// size of the file
			struct	_stat	fileInfo;
			
			if ( _stat(pAttrs->astrName, &fileInfo) != - 1)
			{
		
				pAttrs->mtime = fileInfo.st_mtime;
			}
				
		}
                 
	}
	
	return( boolSuccess );
}
#else
bool ChUtil::ReadDirectory( void * pDirHandle, pChFileAttrs pAttrs, chuint flRequest )
{
    bool      	boolSuccess = true;
    pChDirInfo 	pfData = (pChDirInfo)pDirHandle;  

	if (!pfData->boolFirst)
	{
		if ( pfData->hFindFirst )
		{
			boolSuccess = (FindNextFile(pfData->hFindFirst, &pfData->findFile) != FALSE);
		}

	}
	else
	{
		pfData->boolFirst = false;
	}  
	
	if (boolSuccess)
	{
		if (pfData->findFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			pAttrs->uFileType = ChUtil::typeDir;
	
		}
		else
		{
			pAttrs->uFileType = ChUtil::typeFile;
		}

		if ( flRequest & reqTime )
		{
			// last update time	
			pAttrs->mtime = pfData->findFile.ftLastWriteTime;
		}
		if ( flRequest & reqSize )
		{
			// size of the file
			pAttrs->luSize = pfData->findFile.nFileSizeLow;
		}
			

   		if ( flRequest & reqPath )
		{
	        lstrcpy( pAttrs->astrName,  pfData->astrPath );

			int suDirPathLen = lstrlen( pAttrs->astrName ) - 1;
			while ( suDirPathLen && pAttrs->astrName[suDirPathLen] != TEXT( '\\' ))
			{
				--suDirPathLen;
			}
	                                     /* Path to the file */
			lstrcpy( &pAttrs->astrName[suDirPathLen + 1], pfData->findFile.cFileName );
		}
	}

	return( boolSuccess );
}
#endif  // CH_ARCH_16

#else
bool ChUtil::ReadDirectory( void * pDirHandle, pChFileAttrs pAttrs, chuint flRequest )
{
		cerr << "XXX" << __FILE__ << ":" << __LINE__ << endl;	
	return false;
}
#endif // CH_MSW

bool ChUtil::CloseDirectory( void * pDirHandle )
{
	bool		boolSuccess = true;

	#if defined( CH_MSW )
		#if !defined( CH_ARCH_32 ) 
		{
			if ( pDirHandle )
			{
				delete (pChDirInfo)pDirHandle;
			} 
		}
		#else
		{  
			if ( pDirHandle )
			{
				pChDirInfo pDir = (pChDirInfo)pDirHandle;
			
				if ( pDir->hFindFirst )
				{
					boolSuccess = (FindClose( pDir->hFindFirst ) != FALSE);
				}
				else
				{
					boolSuccess = true;
				}
				delete pDir; 
			}
		}
		#endif 
	#else
		if ( pDirHandle )
		{
			delete (pChDirInfo)pDirHandle;
		}
		cerr << "XXX" << __FILE__ << ":" << __LINE__ << endl;	
	#endif

	return ( boolSuccess );
}

#if defined( CH_MSW )
#if defined( CH_ARCH_16 )
int ChUtil::CompareFileTime( chuint32 time1, chuint32 time2 )  
{
	return ( time1 < time2 ) ?  -1 : ( (time1 > time2 ) ?  1 : 0 );
}
#else
int ChUtil::CompareFileTime( FILETIME& time1, FILETIME& time2 )  
{
	return(::CompareFileTime( &time1, &time2 ));
}
#endif
#else
int ChUtil::CompareFileTime( chuint32 time1, chuint32 time2 )  
{
	return ( time1 < time2 ) ? -1 : ( (time1 > time2 )? 1 : 0 );
}
#endif



void ChUtil::GetAppDirectory( ChString& strDir )
{
	if (!m_directorySet)
	{										/* We need to construct the app
												directory and cache it */
		char	strPath[MAX_PATH];

		m_directorySet = true;
		#if defined( CH_MSW )
		{									// Start with the application path
			int		iLastSeparator;

			#if defined( CH_ARCH_32 )
			{
				::GetModuleFileName( ::GetModuleHandle( 0 ), strPath,
										MAX_PATH );
			}
			#else
			{
				CWinApp*	pApp = AfxGetApp();

				::GetModuleFileName( pApp->m_hInstance, strPath, MAX_PATH );
			}
			#endif

			m_strAppDirectory = strPath;
												// Strip out the app name

			iLastSeparator = m_strAppDirectory.ReverseFind( TEXT( '\\' ) );
			if (iLastSeparator >= 0)
			{
				m_strAppDirectory =
						m_strAppDirectory.Left( iLastSeparator + 1 );
			}
		}
		#elif defined( CH_UNIX )
		{
		    #if defined( CH_CLIENT )
			{
				m_strAppDirectory = m_strPuebloDirectory + "/";
			}
			#endif
		}
		#else
		{
			#error "ChUtil::GetAppDirectory : OS not defined"
		}
		#endif
	}

	strDir = m_strAppDirectory;
}


/*----------------------------------------------------------------------------

	FUNCTION		||	CreateDirectoryTree

	strPath			||	Path or name of directory to create

	RETURN			||	true if successful, else false

-------------------------------DESCRIPTION-----------------------------------

	Creates a directory with the given name.

----------------------------------------------------------------------------*/

bool ChUtil::CreateDirectoryTree( const ChString& strPath )
{
	char	astrDir[MAX_PATH + 1];
	bool	boolSkipCheck = 0;
	char*	pstrSubPath;

	lstrcpy( astrDir, strPath );
	pstrSubPath = strchr( astrDir, PATH_SEPARATOR_CHAR );

	while (pstrSubPath)
	{
		pstrSubPath = strchr( pstrSubPath + 1, PATH_SEPARATOR_CHAR );

        if ( pstrSubPath )
        {
			*pstrSubPath = 0;
		}

	  	if (boolSkipCheck || !ChUtil::FileExists( astrDir ) )
	  	{
	  		boolSkipCheck = true;

	  		#if defined( CH_ARCH_16 )
			{
		  		if ( (_mkdir( astrDir )) )
				{							/* If the create failed, return
												the error code */
					return( false );
				}
			}
			#else
			{
				if (!CreateDirectory( astrDir, 0 ))
				{
					return( false );
				}
			}
			#endif

			if (pstrSubPath)
			{
			    *pstrSubPath = PATH_SEPARATOR_CHAR;
			}
	  	}
	  	else
	  	{									// Next sub-path
	  		if (pstrSubPath)
	  		{
	  			*pstrSubPath = PATH_SEPARATOR_CHAR;
	  		}
	  	}
	}

	return( true );
}
	

/*----------------------------------------------------------------------------

	FUNCTION		||	GetSystemType

	RETURN			||	Returns the type of system we are running under

-------------------------------DESCRIPTION-----------------------------------

----------------------------------------------------------------------------*/

int ChUtil::GetSystemType()
{
	if (m_iSysType != CH_SYS_UNKNOWN)
	{
		return m_iSysType;
	}

	#if defined( CH_MSW )
	{
		#if	defined( WIN32 )
		OSVERSIONINFO osInfo;
		osInfo.dwOSVersionInfoSize = sizeof( osInfo );
		GetVersionEx( &osInfo );

		switch( osInfo.dwPlatformId )
		{

			case VER_PLATFORM_WIN32s :
			{
				m_iSysType = CH_SYS_WIN32S;
				break;
			}
			case VER_PLATFORM_WIN32_WINDOWS :
			{
				m_iSysType = CH_SYS_WIN95;
				break;
			}
			case VER_PLATFORM_WIN32_NT	 :
			default :
			{
				m_iSysType = CH_SYS_WINNT;
				break;
			}

		}
		#else	// defined( WIN32 )
		{									/* We need to update this if we
												write 16 bit code to determine
												if we are running under NT or
												Win95 */
			m_iSysType = CH_SYS_WIN3X;
		}
		#endif	// defined( WIN32 )
	}
	#else	// defined( CH_MSW )
	{
		m_iSysType = CH_SYS_UNIX;			/* The only other platform we
												support for now */
	}
	#endif	// defined( CH_MSW )

	return m_iSysType;
}
	

/*----------------------------------------------------------------------------

	FUNCTION		||	GetSystemProperties

	RETURN			||	Returns flags indicating the properties of the
						system we are running under.

-------------------------------DESCRIPTION-----------------------------------

The following flags may be returned:

	CH_PROP_WIN95			-- Windows95 GUI.
	CH_PROP_LONG_FILENAMES	-- Long file names supported.
	CH_PROP_MULTITHREADED	-- Multithreaded.

----------------------------------------------------------------------------*/

chflag32 ChUtil::GetSystemProperties()
{
	if ((chflag32)-1 != m_flSysProps)
	{
		return m_flSysProps;
	}
	else
	{
		chflag32	flResult = 0;

		#if defined( CH_MSW )
		{
			switch( GetSystemType() )
			{
				case CH_SYS_WIN95:
				{
					flResult = CH_PROP_WIN95 | CH_PROP_LONG_FILENAMES |
									CH_PROP_MULTITHREADED;
					break;
				}

				case CH_SYS_WINNT:
				{
					OSVERSIONINFO osInfo;
					osInfo.dwOSVersionInfoSize = sizeof( osInfo );
					GetVersionEx( &osInfo );

					if ( osInfo.dwMajorVersion > 3 )
					{
						flResult = CH_PROP_WIN95 | CH_PROP_LONG_FILENAMES |
										CH_PROP_MULTITHREADED;
					}
					else
					{
						flResult = CH_PROP_LONG_FILENAMES | CH_PROP_MULTITHREADED;
					}
					break;
				}

				case CH_SYS_WIN3X:
				case CH_SYS_WIN32S:
				{
					flResult = 0;
					break;
				}

				default:
				{
					break;
				}
			}
		}
		#else	// defined( CH_MSW )
		{
			flResult = CH_PROP_LONG_FILENAMES ; // | CH_PROP_MULTITHREADED;
		}
		#endif	// defined( CH_MSW )
											// Cache the result
		m_flSysProps = flResult;

		return flResult;
	}
}


/*----------------------------------------------------------------------------

	FUNCTION		||	PointsToPixels

	pDC				||	Pointer to a device context.

	iPoints			||	Point size desired.  There are 72 points / inch.

	boolVertical	||	true if vertical screen resolution should be used,
						and false otherwise.  This parameter doesn't need
						to be explicitly passed.

	RETURNS			||	The pixel size corresponding to the desired point
						size.

------------------------------------------------------------------------------

	This utility function will return the correct pixel size for a desired
	point size.  The function's primary use is in obtaining font heights.
	NOTE that the size returned is based on vertical pixel resolution, and'
	may not be acurate for horizontal resolution.  To use horizontal
	resolution, pass the third parameter as false.

----------------------------------------------------------------------------*/

#if defined( CH_MSW )

int ChUtil::PointsToPixels( CDC *pDC, int iPoints, bool boolVertical )
{
	int		iPixelsPerInch;
	int		iPixels;

	iPixelsPerInch = boolVertical ? pDC->GetDeviceCaps( LOGPIXELSY ) :
									pDC->GetDeviceCaps( LOGPIXELSX );

	iPixels = (int)(((long)iPixelsPerInch * (long)iPoints) / 72L);

	return iPixels;
}

#endif	// defined( CH_MSW )


/*----------------------------------------------------------------------------
	Windows 16-bit string functions
----------------------------------------------------------------------------*/

#if defined( CH_MSW ) && defined( CH_ARCH_16 )

CH_GLOBAL_LIBRARY( void )
TrimLeft( ChString& strWork )
{
	int nDataLength = strWork.GetLength();
	// find first non-space character
	char* pstrBuf = strWork.GetBuffer( nDataLength );
	char* pstrStart = pstrBuf;
	while ( isspace(*pstrBuf) )
		pstrBuf = pstrBuf++;

	// fix up data and length
	nDataLength = nDataLength - (pstrBuf - pstrStart);
	ChMemMove( pstrStart, pstrBuf, nDataLength + 1);

	strWork.ReleaseBuffer();
}


CH_GLOBAL_LIBRARY( void )
TrimRight( ChString& strWork )
{
	int nDataLength = strWork.GetLength();
	char* pstrBuf = strWork.GetBuffer( nDataLength );
	char* pstrLast = NULL;

	while (*pstrBuf != '\0')
	{
		if ( isspace(*pstrBuf))
		{
			if (pstrLast == NULL)
				pstrLast = pstrBuf;
		}
		else
			pstrLast = NULL;
		pstrBuf = pstrBuf++;
	}

	if (pstrLast != NULL)
	{
		// truncate at trailing space start
		*pstrLast = '\0';
	}
	strWork.ReleaseBuffer();
}

#endif // defined( CH_MSW ) && defined( CH_ARCH_16 )


void ChUtil::EncryptString( ChString& strSource, bool boolEncrypt,
							char cDelimiter )
{
	ASSERT( cDelimiter != ENCRYPT_ESCAPE );

	if (!boolEncrypt && IsOldEncryptString( strSource ))
	{
		OldEncryptString( strSource, boolEncrypt, cDelimiter );
	}
	else
	{
		if (boolEncrypt)
		{
			DoEncrypt( strSource, cDelimiter );
		}
		else
		{
			DoDecrypt( strSource );
		}
	}
}


void ChUtil::EncodeAttributeString( const ChString& strSource,
									ChString& strDest )
{
	int		iIndex;

	strDest = strSource;

	while ((iIndex = strDest.Find( '"' )) != -1)
	{
		strDest = strDest.Left( iIndex ) + "&quot;" +
					strDest.Mid( iIndex + 1 );
	}
}


/*----------------------------------------------------------------------------
	ChUtil protected methods
----------------------------------------------------------------------------*/

void ChUtil::DoEncrypt( ChString& strSource, char cDelimiter )
{
	int				iKeyLen = lstrlen( m_pstrEncryptKey );
	int				iSrcIndex = 0;
	int				iKeyIndex = 0;
	ChString			strResult;
											/* Set the flag indicating that
												this string is simply
												encrypted */
	AppendChar( strResult, ENCRYPT_STYLE_SIMPLE );

	while (iSrcIndex < strSource.GetLength())
	{
		char	cEncrypt =  m_pstrEncryptKey[iKeyIndex % iKeyLen];
		char	chNewChar = strSource[iSrcIndex] ^ cEncrypt;
		bool	boolAppend = true;

		if (cDelimiter && (cDelimiter == chNewChar))
		{
											/* Escape the character that they
												want to use as their delim */

			AppendChar( strResult, chNewChar, true );
			boolAppend = false;
		}
		else if (ENCRYPT_ESCAPE == chNewChar)
		{									// Escape the escape character

			AppendChar( strResult, ENCRYPT_ESCAPE );
		}

		if (boolAppend)
		{
			AppendChar( strResult, chNewChar );
		}

		iSrcIndex++;
		iKeyIndex++;
	}

	strSource = strResult;
}


void ChUtil::DoDecrypt( ChString& strSource )
{
	ChString			strResult;
	const char*		pstrSource = strSource;
	char			cFirst;
											/* The first character indicates if
												the string is encrypted, and
												which method was used */
	cFirst = ReadChar( &pstrSource );

	if (ENCRYPT_STYLE_SIMPLE == cFirst)
	{
		int		iKeyLen = lstrlen( m_pstrEncryptKey );
		int		iKeyIndex = 0;
											/* Loop through the source
												decrypting */
		while (*pstrSource)
		{
			char	cDecrypt = m_pstrEncryptKey[iKeyIndex % iKeyLen];
			char	cCurr;
			char	chNewChar;

			cCurr = ReadChar( &pstrSource );
			chNewChar = cCurr ^ cDecrypt;
			strResult += chNewChar;

			iKeyIndex++;
		}

		strSource = strResult;
	}
}


void ChUtil::AppendChar( ChString& strDest, const char cAppend,
							bool boolForceHex )
{
	if (isprint( cAppend ) && ('\t' != cAppend) && (' ' != cAppend) &&
			!boolForceHex)
	{
											// Append printable characters
		strDest += cAppend;
	}
	else
	{										/* Append the hex representation of
												nonprintable characters */
		char	cBuffer[6];

		sprintf( cBuffer, "%02x", (int)cAppend );
		strDest += "\\x";
		strDest += cBuffer;
	}
}


char ChUtil::ReadChar( const char** ppstrSource )
{
	char	cResult = **ppstrSource;

	(*ppstrSource)++;

	if (ENCRYPT_ESCAPE == cResult)
	{										/* Escape character found on
												decrypt -- read the next
												character */
		cResult = **ppstrSource;

		switch (cResult)
		{
			case ENCRYPT_HEX_CODE:
			{								/* Hex constant -- read the two
												digits */
				int		iVal;

				(*ppstrSource)++;
				if (sscanf( *ppstrSource, "%2x", &iVal ))
				{
											// Successfully read!
					cResult = (char)iVal;
											// Skip two characters
					(*ppstrSource)++;
					if (**ppstrSource) (*ppstrSource)++;
				}
				else
				{							/* This is unexpected, but I don't
												want to assert */
					cResult = '?';
				}
				break;
			}

			default:
			{
				(*ppstrSource)++;
				break;
			}
		}
	}

	return cResult;
}


bool ChUtil::IsOldEncryptString( ChString& strSource )
{
	const char*		pstrSource = strSource;
	bool			boolOldEncrypt = false;

	while (!boolOldEncrypt && (*pstrSource != 0))
	{
		if (!isprint( *pstrSource ))
		{
			boolOldEncrypt = true;
		}
		pstrSource++;
	}

	return boolOldEncrypt;
}


void ChUtil::OldEncryptString( ChString& strSource, bool boolEncrypt,
								char cDelimiter )
{
	const ChString	strKey = "Spaulding Grey is the official parrot of "
								"Andromedia Incorporated";
	const char		cEscape = '\\';
	const char		cInternalDelim = 'd';

	int				iKeyLen = strKey.GetLength();
	int				iSrcIndex = 0;
	int				iKeyIndex = 0;
	ChString			strResult;

	ASSERT( cDelimiter != cEscape );
	ASSERT( cDelimiter != cInternalDelim );

	while (iSrcIndex < strSource.GetLength())
	{
		char	cEncrypt = strKey[iKeyIndex % iKeyLen];
		char	chNewChar = strSource[iSrcIndex] ^ cEncrypt;
		bool	boolAppend = true;

		if (cDelimiter)
		{
			if (boolEncrypt)
			{
				if (cEscape == chNewChar)
				{							// Escape the escape character
					strResult += cEscape;
				}
				else if (cDelimiter == chNewChar)
				{
											/* Escape the character that they
												want to use as their delim */
					strResult += cEscape;
					strResult += cInternalDelim;
					boolAppend = false;
				}
			}
			else if (cEscape == strSource[iSrcIndex])
			{
											/* Escape character found on
												decrypt -- go to the next
												character */
				iSrcIndex++;

				if (strSource[iSrcIndex])
				{
					if (cInternalDelim == strSource[iSrcIndex])
					{
											/* Internal delimiter found, so
												substitute it for the users
												delimiter */
						chNewChar = cDelimiter;
					}
					else
					{						/* Otherwise just decrypt the
												next character */

						chNewChar = strSource[iSrcIndex] ^ cEncrypt;
					}
				}
			}
		}

		if (boolAppend)
		{
			strResult += chNewChar;
		}

		iSrcIndex++;
		iKeyIndex++;
	}

	strSource = strResult;
}

int	 ChUtil::LoadString( UINT nID, ChString& strBuffer )
{
	LPTSTR 	pstrBuffer = strBuffer.GetBuffer( 255 );
	ASSERT( pstrBuffer );

	HINSTANCE hInst;
	#if !defined(CH_PUEBLO_PLUGIN) && !defined(CH_STATIC_LINK )
	hInst = PuebloDLL.hModule;
	#else
	hInst = AfxGetInstanceHandle( );
	#endif

	int nLen;
	if ((nLen = ::LoadString( hInst, nID, pstrBuffer, 255)) != 0)
	{  
		strBuffer.ReleaseBuffer();
		return nLen;
	}

	strBuffer.ReleaseBuffer();

	return 0;

}

void ChUtil::HtmlAddNameValuePair( ChString& strData, const ChString& strName, const ChString& strValue )
{
	// Format the value
	ChString strTmp;

	int i = 0;
	while( i < strValue.GetLength() )
	{

		if ( strValue[i] == TEXT( ' ' ) )
		{
			strTmp += TEXT( '+' );
		}
//		else if ( strValue[i] == TEXT( '\r' )  || strValue[i] == TEXT( '\n' ) )
//		{ // do  not encode newlines 
//			strTmp += strValue[i];
//		}
		else if ( !isalnum( strValue[i] ) )
		{
			char	strNum[10];

			strTmp += TEXT( '%' );
			sprintf( strNum, "%02X", (int)strValue[i] );
			strTmp += strNum;
		}
		else
		{
			strTmp += strValue[i];
		}
		i++;
	}

	// Format the name value
	if( !strData.IsEmpty() )
	{
		strData += '&';
	}
	strData += strName;
	strData += TEXT( '=' );
	strData += strTmp;
}
void ChUtil::HtmlAddNameValuePair( ChString& strData, const ChString& strName, int iValue )
{
	ChString strValue;

	strValue.Format( "%d", iValue );

	HtmlAddNameValuePair( strData, strName, strValue );
}




/*----------------------------------------------------------------------------
	ChTmpFileManager class
----------------------------------------------------------------------------*/

ChTmpFileManager::~ChTmpFileManager()		
{
	ChPosition pos = m_tmpFileList.GetHeadPosition();
	while( pos )
	{
		ChString *pstrTmpFile = (ChString*)m_tmpFileList.GetNext( pos );

		::DeleteFile( *pstrTmpFile );

		delete pstrTmpFile;
	}
}

void ChTmpFileManager::Empty()		
{

	ChPosition pos = m_tmpFileList.GetHeadPosition();
	while( pos )
	{
		ChString *pstrTmpFile = (ChString*)m_tmpFileList.GetNext( pos );

		::DeleteFile( *pstrTmpFile );

		delete pstrTmpFile;
	}

	m_tmpFileList.Empty();

}

////////////////////////////////////////////////////////////////////////
///////// HACK for reality lab unload bug


void ChUtil::AddModuleToLoadList( const ChString& strModule )
{
	bool boolAdd = true;
	ChPosition pos = dllLoadMgr.GetLoadList().GetHeadPosition();
	while( pos )
	{
		ChDllLoadInfo *pInfo = (ChDllLoadInfo*)dllLoadMgr.
									GetLoadList().GetNext( pos );

		if ( strModule.CompareNoCase( pInfo->GetModuleName() ) == 0 )
		{
			boolAdd = false;
			break;
		}

	}

	if ( boolAdd )
	{
		HMODULE hLibrary = ::LoadLibrary( strModule );
		if ( hLibrary )
		{
			ChDllLoadInfo* pInfo = new ChDllLoadInfo(strModule, hLibrary );
			ASSERT( pInfo );
			dllLoadMgr.GetLoadList().AddTail( (chparam)pInfo );
		}
	}
}


ChDllLoadManager::~ChDllLoadManager()		
{
	ChPosition pos = dllLoadMgr.GetLoadList().GetHeadPosition();
	while( pos )
	{
		ChDllLoadInfo *pInfo = (ChDllLoadInfo*)dllLoadMgr.
									GetLoadList().GetNext( pos );

		::FreeLibrary( pInfo->GetLibrary() );

		delete pInfo;
	}
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChUtil::GetNextHdr

------------------------------------------------------------------------------
Get the next HTTP header to process (UE: moved from ChHTTPInfo)
----------------------------------------------------------------------------*/
#define CR '\x0D'
#define LF '\x0A'
bool ChUtil::GetNextHeaderString( char* &pstrHdr, char* &pstrData, char* &pstrNext, int iDelimType ) {
	if (!pstrHdr) {
		return false;
	}

	pstrNext = strchr( pstrHdr, TEXT( ':' ));

	if ( !pstrNext ) {
		return false;
	}
	*pstrNext = 0;

											// Remove any white space
	while( *pstrHdr == TEXT( ' ' )) {
		pstrHdr++;
	}

	pstrData = pstrNext + 1;
	pstrNext = strchr( pstrData, LF );

	if (DELIM_STANDARD == iDelimType) {
		*(pstrNext - 1) = 0;				// remove CR
	}

	*pstrNext = 0;							// remove LF
	pstrNext++;								// next command

	if (DELIM_STANDARD == iDelimType) {
		if (*pstrNext == CR && *(pstrNext + 1) == LF) {
			pstrNext = 0;
		}
	} else if (LF == *pstrNext) {	// this is the last command
	 	pstrNext = 0;
	}

	return true;
}


// $Log$
