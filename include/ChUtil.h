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

	This file contains the interface for utility functions.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHUTIL_H_ )
#define _CHUTIL_H_

#if defined( CH_MSW )
#if defined( CH_ARCH_16 )
#include <dos.h>
#endif
#endif	

#include <ChList.h>		 


#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif


#if defined( CH_MSW ) && defined( CH_ARCH_32 )
											/* Disable warnings about non-
												exported classes for MSW */
	#pragma warning( disable: 4275 )
	#pragma warning( disable: 4251 )

#endif	// defined( CH_MSW )

#ifdef CH_MSW
#define PATH_SEPARATOR_CHAR		TEXT( '\\' )
#elif defined( CH_UNIX )
#define PATH_SEPARATOR_CHAR		'/'
#else
#error Unknown system
#endif


#if defined( CH_UNIX )

	typedef struct _string_resource
		{
			int resource_val;
			char *resource_string;

		} string_resource;

#endif	// defined( CH_UNIX )


typedef struct								/* File attributes */
{
    	chuint32       	flMode;                     /* Open mode */

		char     		astrPath[ 256 ];    	/* Dir search path */

		bool			boolFirst;
		#if defined( CH_MSW )
		#if !defined(CH_ARCH_32)

		int				hFindFirst;					/* Handle returned by find first */
		struct _find_t	findFile;
		#else
		HANDLE			hFindFirst;					/* Handle returned by find first */
		WIN32_FIND_DATA	findFile;
		#endif
		#endif

} ChDirInfo, FAR * pChDirInfo;


typedef struct								/* File attributes */
{
	int				uFileType;

	#if defined( CH_MSW )
   	#if defined( CH_ARCH_16 )
	chuint32		mtime;
	#else
	FILETIME		mtime;
	#endif
	#elif defined( CH_UNIX )
	chuint32		mtime;
	#else
	#error Unknown platform
	#endif

	chuint32		luSize;
	char			astrName[256];

} ChFileAttrs, FAR *pChFileAttrs;    

class ChClientInfo;


class ChTmpFileManager
{
	public :
		ChTmpFileManager()		{}
		~ChTmpFileManager();
		// Methods
		void Empty();

		void Add( const ChString& strFile ) 
			{  
				ChString *pNew = new ChString( strFile );
				m_tmpFileList.AddTail( (chparam)pNew );
			}
	private :
		ChParamList 	m_tmpFileList;
	
};




/*----------------------------------------------------------------------------
	ChUtil class
----------------------------------------------------------------------------*/

#define DELIM_STANDARD			1
#define DELIM_NONSTANDARD		2

class CH_EXPORT_CLASS ChUtil
{
	public:
											/* The following function loads a
												specified resource.  On MSW,
												the first param should be a
												hModule.  On Unix, it should
												be a (string_resource*). */
		enum { typeDir = 1, typeFile };
		enum { reqTime = 0x01, reqSize = 0x02, reqPath = 0x04 };

		static bool Load( chparam resources, chint32 lKey, ChString& strData );
		static int	LoadString( UINT nID, ChString& strBuffer );

		static bool FileExists( const char* pstrFilename );
		static void GetAppDirectory( ChString& strDir );
		static bool SetFileToCurrentTime( const char* pstrFilename );
		static void* OpenDirectory( const char* pstrPath,
									const char* pstrFilter, chuint32 flMode );
		static bool ReadDirectory( void* pDirHandle, pChFileAttrs pAttrs,
									chuint flRequest );
		static bool CloseDirectory( void* pDirHandle );
		static bool CreateDirectoryTree( const ChString& strPath ); 
		static void	AddFileToTempList( const ChString& strFile );
		static void	EmptyTempFileList();
		static void AddModuleToLoadList( const ChString& strModule );

		static bool GetNextHeaderString( char* &pstrHdr, char* &pstrData, char* &pstrNext, int iDelimType );

		#if defined( CH_MSW )
			#if defined( CH_ARCH_16 )
			static int CompareFileTime( chuint32 time1, chuint32 time2 );
			#else
			static int CompareFileTime( FILETIME& time1, FILETIME& time2 );
			#endif
		#else

		static int CompareFileTime( chuint32 time1, chuint32 time2 );

		#endif

		static bool GetTempFileName( ChString& strTmpFile,
										const char* pstrTempPath = 0,
										const char* pstrPrefix = 0,
										const char* pstrExtn = 0 );

		static void HtmlAddNameValuePair( ChString& strData, const ChString& strName, const ChString& strValue );
		static void HtmlAddNameValuePair( ChString& strData, const ChString& strName, int iValue );


		#if defined( CH_UNIX )

		static const ChString& GetPuebloDir() { return m_strPuebloDirectory; }
		static void SetPuebloDir( const ChString& strPuebloDir )
				{
					m_strPuebloDirectory = strPuebloDir;
				}

		#endif
		
		static int GetSystemType();			/* Returns the type of system we
												are running under */

											/* Returns the properties of the
												system we're running under */
		static chflag32 GetSystemProperties();

		static void EncryptString( ChString& strSource, bool boolEncrypt,
									char cDelimiter = 0 );
		static void EncodeAttributeString( const ChString& strSource,
											ChString& strDest );

		#if defined( CH_MSW )

		static int PointsToPixels( CDC *pDC, int iPoints,
									bool boolVertical = true );

		#endif	// defined( CH_MSW )

	protected:
											// This class is never constructed
		ChUtil() {}

		static void DoEncrypt( ChString& strSource, char cDelimiter );
		static void DoDecrypt( ChString& strSource );
		static void AppendChar( ChString& strDest, const char cAppend,
								bool boolForceHex = false );
		static char ReadChar( const char** ppstrSource );

		static bool IsOldEncryptString( ChString& strSource );
		static void OldEncryptString( ChString& strSource, bool boolEncrypt,
										char cDelimiter );

	protected:
		static ChString		m_strAppDirectory;
		static bool			m_directorySet;
		
	private :
		static int			m_iSysType;
		static chflag32		m_flSysProps;

		#if defined( CH_UNIX )

		static ChString		m_strPuebloDirectory;

		#endif

		static const char*			m_pstrEncryptKey;
		static ChTmpFileManager 	m_tmpFileMgr;
};


/*----------------------------------------------------------------------------
	Windows 16-bit string functions
----------------------------------------------------------------------------*/

#if defined( CH_MSW ) && defined( CH_ARCH_16 )

CH_EXTERN_LIBRARY( void )
TrimLeft( ChString& strWork );

CH_EXTERN_LIBRARY( void )
TrimRight( ChString& strWork );

#endif

// $Log$

#endif	// !defined( _CHUTIL_H_ )
