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

	This file contains the implementation of the ChHTTPConn class, used to
	manage a connection for downloading modules and data from the server.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <ChTypes.h>
#include <ChHTTP.h>
#include <ChList.h>

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	Types
----------------------------------------------------------------------------*/

class ChMimeInfo
{
	public :
		ChMimeInfo( int iType, const ChString& strMimeType, 
								const ChString& strFileExtent, const ChString& strFileDesc ) :
							 m_iType( iType ),
							 m_strMimeType( strMimeType ),
							 m_strFileExtent( strFileExtent ),
							 m_strFileDesc( strFileDesc ) 
							 {
							 }
		int 		  GetType()			{ return m_iType; }
		const ChString& GetMimeType()		{ return m_strMimeType; }
		const ChString& GetFileExtn()		{ return m_strFileExtent; }
		const ChString& GetFileOpenDesc()	{ return m_strFileDesc; }

		void  SetFileOpenDesc( const ChString& strDesc )			{ m_strFileDesc = strDesc; }

		

	private :
		int			m_iType;
		ChString 	 	m_strMimeType;
		ChString 	 	m_strFileExtent;
		ChString 	 	m_strFileDesc;		 
};

class ChMimeListManager
{
	public :
		ChMimeListManager();
		~ChMimeListManager();
		// Methods
		void Init();
		void Term();

		void AddMimeType( const ChString& strMime, const ChString& strFileExtn, 
							const ChString& strDesc ); 
		ChParamList& GetMimeList()  		
				{ 
					return m_mimeList; 
				}

	private :
		ChParamList 	m_mimeList;
	
};


/*----------------------------------------------------------------------------
	static values
----------------------------------------------------------------------------*/
static ChMimeListManager	mimeLstMgr;
static bool					boolInitializedPlugin = false;

/*----------------------------------------------------------------------------
	ChMimeListManager implementation
----------------------------------------------------------------------------*/

ChMimeListManager::ChMimeListManager()
{

}
void ChMimeListManager::Init()

{											// Add the default types
	if ( m_mimeList.GetCount() )
	{   // Already initialized;
		return;	
	}

	ChMimeInfo* pInfo;

	pInfo = new ChMimeInfo( ChHTTPConn::typeHTML, MIME_HTML, "htm", "HTML(*.html)" );
	ASSERT( pInfo );
	m_mimeList.AddTail( (chparam)pInfo );

	pInfo = new ChMimeInfo( ChHTTPConn::typeHTML, MIME_HTML, "html", "HTML(*.htm)" );
	ASSERT( pInfo );
	m_mimeList.AddTail( (chparam)pInfo );

	pInfo = new ChMimeInfo( ChHTTPConn::typeHTML, MIME_HTML2, "htm", "HTML(*.htm)" );
	ASSERT( pInfo );
	m_mimeList.AddTail( (chparam)pInfo );

	pInfo = new ChMimeInfo( ChHTTPConn::typeText, MIME_TEXT, "txt", "Text(*.txt)" );
	ASSERT( pInfo );
	m_mimeList.AddTail( (chparam)pInfo );	

	pInfo = new ChMimeInfo( ChHTTPConn::typeGIF, MIME_GIF, "gif", "GIF Image(*.gif)" );
	ASSERT( pInfo );
	m_mimeList.AddTail( (chparam)pInfo );

	pInfo = new ChMimeInfo( ChHTTPConn::typeJPEG, MIME_JPEG, "jpg", "JPEG Image(*.gif)" );
	ASSERT( pInfo );
	m_mimeList.AddTail( (chparam)pInfo );
	
	pInfo = new ChMimeInfo( ChHTTPConn::typeBMP, MIME_BMP, "bmp", "Windows Bitmap(*.bmp)" );
	ASSERT( pInfo );
	m_mimeList.AddTail( (chparam)pInfo );
	
	pInfo = new ChMimeInfo( ChHTTPConn::typeMidi, MIME_MIDI, "mid", "MIDI(*.mid)" );
	ASSERT( pInfo );
	m_mimeList.AddTail( (chparam)pInfo );

	pInfo = new ChMimeInfo( ChHTTPConn::typeMidi, MIME_MIDI1, "midi", "MIDI(*.midi)" );
	ASSERT( pInfo );
	m_mimeList.AddTail( (chparam)pInfo );
	
	pInfo = new ChMimeInfo( ChHTTPConn::typeMidi, MIME_MIDI2, "mid", "MIDI(*.mid)" );
	ASSERT( pInfo );
	m_mimeList.AddTail( (chparam)pInfo );
	
	pInfo = new ChMimeInfo( ChHTTPConn::typeWave, MIME_WAVE, "wav", "Wave(*.wav)" );
	ASSERT( pInfo );
	m_mimeList.AddTail( (chparam)pInfo );
	
	pInfo = new ChMimeInfo( ChHTTPConn::typeVRML, MIME_VRML, "wrl", "VRML Worlds (*.wrl)" );
	ASSERT( pInfo );
	m_mimeList.AddTail( (chparam)pInfo );

	pInfo = new ChMimeInfo( ChHTTPConn::typeVox, MIME_VOX, "vox", "Vox files (*.vox)" );
	ASSERT( pInfo );
	m_mimeList.AddTail( (chparam)pInfo );

	pInfo = new ChMimeInfo( ChHTTPConn::typeWorld, MIME_WORLD, "pbl", "Pueblo World files (*.pbl)" );
	ASSERT( pInfo );
	m_mimeList.AddTail( (chparam)pInfo );
}


ChMimeListManager::~ChMimeListManager()
{
	Term();
}

void ChMimeListManager::Term()
{
	ChPosition pos = m_mimeList.GetHeadPosition();
	while( pos )
	{
		ChMimeInfo *pInfo = (ChMimeInfo*)m_mimeList.GetNext( pos );

		delete pInfo;
	}

	m_mimeList.Empty();
}


void ChMimeListManager::AddMimeType( const ChString& strMime, const ChString& strFileExtn, 
										const ChString& strDesc ) 
{

	bool boolAdd = true;
	int iType = 0;
	ChPosition pos = m_mimeList.GetHeadPosition();
	while( pos )
	{
		ChMimeInfo *pInfo = (ChMimeInfo*)m_mimeList.GetNext( pos );

		if ( pInfo->GetMimeType() == strMime )
		{
			iType = pInfo->GetType();

			if (pInfo->GetFileExtn() == strFileExtn )
			{ // repalce this  
				boolAdd = false;
				pInfo->SetFileOpenDesc( strDesc );	
				break;
			}
		}

	}

	if ( boolAdd )
	{
		if ( iType == 0 )
		{
			iType = m_mimeList.GetCount() + 1;
		}

		ChMimeInfo* pInfo = new ChMimeInfo( iType, strMime, strFileExtn, strDesc );
		ASSERT( pInfo );
		m_mimeList.AddTail( (chparam)pInfo );
	}

}

/*----------------------------------------------------------------------------

	FUNCTION	||	GetMimeType

	RETURN		||	Returns the mime type string based on the file extension.

----------------------------------------------------------------------------*/

int ChHTTPConn::GetMimeType( const ChString& strType )
{

	// Initialize the list
	mimeLstMgr.Init();


	ChString		strMimeType( strType );
    
	#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	{
		TrimLeft( strMimeType );
		TrimRight( strMimeType );
	}
	#else
	{
		strMimeType.TrimLeft();
		strMimeType.TrimRight();
	}
	#endif
    
	strMimeType.MakeLower();  

	ChPosition pos = mimeLstMgr.GetMimeList().GetHeadPosition();
	while( pos )
	{
		ChMimeInfo *pInfo = (ChMimeInfo*)mimeLstMgr.GetMimeList().GetNext( pos );

		if ( pInfo->GetMimeType() == strMimeType )
		{
			return pInfo->GetType();
		}
	}

	return -1;	
}


/*----------------------------------------------------------------------------

	FUNCTION		||	GetMimeTypeByFileExtn

	RETURN			||	Returns the mime type string based on the file extension

----------------------------------------------------------------------------*/

void ChHTTPConn::GetMimeTypeByFileExtn( const char* pstrFile,
										ChString& strMimeType )
{				  

	// Initialize the list
	mimeLstMgr.Init();

	strMimeType.Empty();

	const char* pstrExtn = strrchr( pstrFile, TEXT( '.' ) );
	if ( 0 == pstrExtn )
	{
		pstrExtn = pstrFile;
	}
	else
	{
		pstrExtn++;
	}

	ChPosition pos = mimeLstMgr.GetMimeList().GetHeadPosition();
	while( pos )
	{
		ChMimeInfo *pInfo = (ChMimeInfo*)mimeLstMgr.GetMimeList().GetNext( pos );



	   	ChString strTemp( pInfo->GetFileExtn() );

		while( !strTemp.IsEmpty()  )
		{
			int iExtnIndex = strTemp.Find( TEXT( ',' ));

			if ( iExtnIndex == -1 )
			{
				if ( strTemp.CompareNoCase( pstrExtn) == 0 )
				{
					strMimeType = pInfo->GetMimeType();
					return;
				}
				else
				{
					break;
				}
			}
			else
			{
			  	ChString strExtn( strTemp.Left( iExtnIndex ) );
				if ( strExtn.CompareNoCase( pstrExtn) == 0 )
				{
					strMimeType = pInfo->GetMimeType();
					return;
				}

				strTemp = strTemp.Right( strTemp.GetLength() - ( iExtnIndex + 1) );
			}
		}
	}
}
/*----------------------------------------------------------------------------

	FUNCTION		||	GetMimeTypeByFileExtn


	RETURN			||	Returns the mime type string based on the file extension

-------------------------------DESCRIPTION-----------------------------------

----------------------------------------------------------------------------*/

void ChHTTPConn::GetFileExtnByMimeType( int iMimeType, ChString& strExtn  )
{				  

	// Initialize the list
	mimeLstMgr.Init();

	strExtn = TEXT( "tmp" );
	
	ChPosition pos = mimeLstMgr.GetMimeList().GetHeadPosition();
	while( pos )
	{
		ChMimeInfo *pInfo = (ChMimeInfo*)mimeLstMgr.GetMimeList().GetNext( pos );

		if ( pInfo->GetType() == iMimeType )
		{
		   	ChString strTemp( pInfo->GetFileExtn() );
			if( !strTemp.IsEmpty()  )
			{
				int iExtnIndex = strTemp.Find( TEXT( ',' ));

				if ( iExtnIndex != -1 )
				{
				  	strExtn = strTemp.Left( iExtnIndex );
				}
				else
				{
					strExtn = strTemp;
				}
			}
			return;
		}

	}
}


void ChHTTPConn::AddMimeType( const ChString& strMime, const ChString& strFileExtn, 
							const ChString& strDesc )
{
	// Initialize the list
	mimeLstMgr.Init();

 	mimeLstMgr.AddMimeType( strMime, strFileExtn, strDesc );
}

int ChHTTPConn::GetFileOpenFilter( ChString& strFilter )
{	 
	// Initialize the list
	mimeLstMgr.Init();

	return 0;
}

void ChHTTPConn::TermMimeManager( )
{	 
	// Initialize the list
	mimeLstMgr.Term();
}

// $Log$
