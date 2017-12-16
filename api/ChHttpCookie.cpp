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

	This file contains the implementation of the Netscape HTTP cookies.

----------------------------------------------------------------------------*/

#include "headers.h"
#include <iostream>
#include <fstream>

#include <ChUtil.h>
#include <ChUrlMap.h>
#include "ChHttpCookie.h"

#if !defined(CH_PUEBLO_PLUGIN)
#include "resource.h"
#else
#include "vwrres.h"
#endif

#include <MemDebug.h>


CH_INTERN_VAR const char* pstrCookieTokens[] =
					{
						TEXT( "domain" ),
						TEXT( "path" ),
						TEXT( "expires" ),
						TEXT( "secure" ),
					};

#define COOKIE_DOMAIN 		0
#define COOKIE_PATH 		1
#define COOKIE_EXPIRES 		2
#define COOKIE_SECURE 		3

#define COOKIE_DELIMITOR			TEXT( '\t' )
#define COOKIE_NEWLINE				TEXT( "\r\n" )
#define COOKIE_FILE					TEXT( "cookie.txt" )


/*----------------------------------------------------------------------------
	ChHttpCookie::ChHttpCookie 
----------------------------------------------------------------------------*/

ChHttpCookie::ChHttpCookie() :
			 m_boolSaved( true )
{
	if ( !GetModuleFileName( NULL, m_strCookieFile.GetBuffer( 512 ), 512 ) )
	{
		m_strCookieFile.ReleaseBuffer();	
		TRACE( "GetModuleFileName function failed !!!!\n" );
		ASSERT( 0 );
	}

	m_strCookieFile.ReleaseBuffer();	
	// path of application, scrpit modules are stored relative to app path
	m_strCookieFile = m_strCookieFile.Left( m_strCookieFile.ReverseFind( TEXT( '\\' ) ) + 1 );
	m_strCookieFile += COOKIE_FILE;

}


		
/*----------------------------------------------------------------------------
	ChHttpCookie::~ChHttpCookie 
----------------------------------------------------------------------------*/

class ChDeleteAll : public	ChVisitor2<ChString, ChCookieList*>
{
	public:
		ChDeleteAll()  {}

		virtual bool Visit( const ChString& strDomain,  ChCookieList* const& pList );
};

bool ChDeleteAll::Visit(  const ChString& strDomain,  ChCookieList* const& pList )
{
	pList->Empty();
	delete (ChCookieList*)pList;
	return true;
}

ChHttpCookie::~ChHttpCookie()
{
	ChDeleteAll delAll;

	m_cookieTree.Infix( delAll );

	m_cookieTree.Erase();
}
		
/*----------------------------------------------------------------------------
	ChHttpCookie::ReadCookieFile 
----------------------------------------------------------------------------*/
bool ChHttpCookie::ReadCookieFile( const char* pstrFile /*= 0 */ )
{
	if ( !pstrFile )
	{
		pstrFile = m_strCookieFile;
	}
	

	std::fstream streamIn( pstrFile, std::ios::in );

	if ( !streamIn.is_open() )
	{
		return false;
	}

	char*  pstrBuffer = new char[4094];
	ASSERT( pstrBuffer );

	int iCount = streamIn.read( pstrBuffer, 4093 ).gcount();

	if ( 0 == iCount )
	{
		delete []pstrBuffer;
		streamIn.close();	
		return false;
	}

	int		iNext = 0;
	ChString 	strLine;

	do
	{

		iNext = ReadLine( pstrBuffer, iCount, iNext, strLine, &streamIn );
		{
			ChString 		strDomain;
			ChCookie 	cookie;

			if ( MakeCookie( strLine, strDomain, cookie ) )
			{
			 	AddCookie( strDomain, cookie );
			}
		}

	}
	while( strLine.GetLength() && iCount );

	delete [] pstrBuffer;

	streamIn.close();	
	return true;
}

	
/*----------------------------------------------------------------------------
	ChHttpCookie::WriteCookieFile 
----------------------------------------------------------------------------*/

class ChWriteCookie : public	ChVisitor2<ChString, ChCookieList*>
{
	public:
		ChWriteCookie( std::fstream* pStream ) : m_pStream( pStream )  {}

		virtual bool Visit( const ChString& strDomain,  ChCookieList* const& pList );
	private :
		std::fstream*  m_pStream;
};

bool ChWriteCookie::Visit(  const ChString& strDomain,  ChCookieList* const& pList )
{
	ChPosition pos = pList->GetHeadPosition();
	while ( pos )
	{
		ChCookie cookie = pList->Get( pos );

		if ( (unsigned long)cookie.GetExpiresValue().GetTime() != 0 )
		{  // Store only if we have a expiration date
			ChString strWrite( strDomain );

			strWrite += COOKIE_DELIMITOR;

			ChCookie cookie = pList->Get( pos );

			strWrite += cookie.GetPath();
			strWrite += COOKIE_DELIMITOR;

			strWrite += cookie.IsSecure() ? TEXT( "TRUE" ) : TEXT( "FALSE" );
			strWrite += COOKIE_DELIMITOR;

			ChString strTime;
			strTime.Format( "%lu", (unsigned long)cookie.GetExpiresValue().GetTime() );
	 		strWrite += strTime;
			strWrite += COOKIE_DELIMITOR;

	 		strWrite += cookie.GetNameValue();
			strWrite += COOKIE_DELIMITOR;

			strWrite += COOKIE_NEWLINE;

			m_pStream->write( strWrite, strWrite.GetLength() );
		}


		pList->GetNext( pos ); 
		
	}

	return true;
}


bool ChHttpCookie::WriteCookieFile( const char* pstrFile /*= 0 */)
{

	if ( !pstrFile )
	{
		pstrFile = m_strCookieFile;
	}

//	fstream streamOut( pstrFile, ios::out | ios::trunc , filebuf::sh_none );
	std::fstream streamOut( pstrFile, std::ios::out | std::ios::trunc );
	// 0 denotes share-exclusion

	if ( !streamOut.is_open() )
	{
	 	return false;
	}

	// Write the file header
	ChString strHdr;

	// Calling loadstring from a DLL is not very safe if the 
	// hResource handle is not set properly

	
	ChUtil::LoadString( IDS_COOKIE_HDR1, strHdr );
	strHdr += COOKIE_NEWLINE;
	streamOut.write( strHdr, strHdr.GetLength() );
	ChUtil::LoadString( IDS_COOKIE_HDR2, strHdr );
	strHdr += COOKIE_NEWLINE;
	streamOut.write( strHdr, strHdr.GetLength() );
	ChUtil::LoadString( IDS_COOKIE_HDR3, strHdr );
	strHdr += COOKIE_NEWLINE;
	streamOut.write( strHdr, strHdr.GetLength() );


	ChWriteCookie writeCookie( &streamOut );

	m_cookieTree.Infix( writeCookie );

	return true;
}

/*----------------------------------------------------------------------------
	ChHttpCookie::GetCookie 
----------------------------------------------------------------------------*/

bool ChHttpCookie::GetCookie( const ChString& strURL, ChString& strCookie, bool& boolSecure )
{

	ChString strDomain;
	ChString strPath;

	strCookie.Empty();
	boolSecure = false;

	if ( !strURL.IsEmpty() )
	{
		ChURLParts urlParts;

		if ( !urlParts.GetURLParts( strURL ) )
		{
			return false;
		}

		ChString strTemp( urlParts.GetHostName() );

		strDomain = strTemp.Right( strTemp.GetLength() - strTemp.Find( TEXT( '.' ) ) );

	 	strPath = TEXT( "/"  );
		strPath +=  (urlParts.GetAbsPath() ?  urlParts.GetAbsPath() : TEXT( "" ));
		if ( urlParts.GetRelPath() )
		{
			if ( strPath.GetLength() &&
					TEXT( '/' ) != strPath[strPath.GetLength()-1] )
			{
				strPath += TEXT( '/' );
			}

			strPath += urlParts.GetRelPath();
		}

			
	}

	strDomain.MakeLower();


	// Do tail match
	int iIndex = strDomain.ReverseFind( TEXT( '.' ) );

	while ( iIndex > 0 )
	{
		iIndex--;	
		while( iIndex && strDomain[iIndex] != TEXT( '.' ) )
		{
			iIndex--;
		}

		ChString strTail( strDomain.Right( strDomain.GetLength() - iIndex ) );
		ChCookieList** ppList;
		ppList = m_cookieTree.Find( strTail );

		if ( ppList && *ppList )
		{
			DoPathMatch( *ppList, strPath, strCookie  );	
		}

	}

   	return !strCookie.IsEmpty();

}


/*----------------------------------------------------------------------------
	ChHttpCookie::SetCookie 
----------------------------------------------------------------------------*/
bool ChHttpCookie::SetCookie( const ChString& strURL, const ChString& strCookie )
{ // What we get here is a HTTP response header Set Cookie result, scan this string
  // and set all the values in our cookie list

 	ChString 			strCookieNameValue;
  	ChTime  		timeExpires;
  	ChString 			strPath( '/' );
  	ChString 			strDomain;
  	bool   			boolSecure = false;
  	LPCSTR 			pstrParse = strCookie;
	bool			boolSuccess	= false;

  	while ( *pstrParse )
  	{
		ChString strType, strValue;
		// Skip white space
  		while ( *pstrParse && isspace( *pstrParse ) )
		{
			++pstrParse;
		}
		// Get the token
		while ( *pstrParse && !isspace( *pstrParse ) &&  *pstrParse != TEXT( '=' ) )
		{
			strType += *pstrParse++; 
		}

		// Skip white space
  		while ( *pstrParse && isspace( *pstrParse ) )
		{
			++pstrParse;
		}

		if ( *pstrParse == TEXT( '=' ) )
		{
			++pstrParse;
			// Skip white space
	  		while ( *pstrParse && isspace( *pstrParse ) )
			{
				++pstrParse;
			}
			// Get the value
			while ( *pstrParse && *pstrParse != TEXT( ';' )  )
			{
				strValue += *pstrParse++; 
			}

		}

		if ( *pstrParse == TEXT( ';' ) )
		{
			pstrParse++;
		}

		// Find the type of token we have
		if ( strType.CompareNoCase( pstrCookieTokens[COOKIE_DOMAIN] ) == 0 )
		{
			if ( strValue[0] != TEXT( '.' ) )
			{
		   		strDomain = TEXT( '.' );
			}
	   		strDomain += strValue;
		}
  		else if ( strType.CompareNoCase( pstrCookieTokens[COOKIE_PATH] ) == 0 )
		{
			if ( !strValue.IsEmpty() )
			{
				if ( strValue[0] != TEXT( '/' ) )
				{
		   			strPath += strValue;
				}
				else
				{
		   			strPath = strValue;
				}
			}
		}
  		else if ( strType.CompareNoCase( pstrCookieTokens[COOKIE_EXPIRES] ) == 0 )
		{
			ChTime time( strValue );
			timeExpires = time;
		}
  		else if ( strType.CompareNoCase( pstrCookieTokens[COOKIE_SECURE] ) == 0 )
		{
	   		boolSecure = true;
		}
		else if ( !strValue.IsEmpty() )
		{
			if ( !strCookieNameValue.IsEmpty() )
			{
			 	strCookieNameValue += TEXT( "; " );	
			}
		 	strCookieNameValue += strType;	
		 	strCookieNameValue += TEXT( '=' );	
		 	strCookieNameValue += strValue;	
		}


  	}
	
	if ( !strCookieNameValue.IsEmpty() )
	{
		// If there is no doamin then use the one in the URL
		if ( strDomain.IsEmpty() )
		{
			ChURLParts urlParts;
	
			urlParts.GetURLParts( strURL );

			ChString strTemp( urlParts.GetHostName() );

			strDomain = strTemp.Right( strTemp.GetLength() - strTemp.Find( TEXT( '.' )) );
				
		}
		if ( IsValidDomain( strDomain ) )
		{
			// Add the cookie
			ChCookie cookie( timeExpires, boolSecure,  strCookieNameValue, strPath );

			strDomain.MakeLower();
			AddCookie( 	strDomain, cookie );  
			boolSuccess = true;
		}
	}
	return boolSuccess;
}

///////////////////////////////////////////////////////////////////////////////
///////////
//////////	   Private methods
//////////
//////////////////////////////////////////////////////////////////////////////
/*----------------------------------------------------------------------------
	ChHttpCookie::AddCookie 
----------------------------------------------------------------------------*/

void ChHttpCookie::AddCookie( const ChString& strDomain, ChCookie& cookie )
{

	ChCookieList** ppList;
	ppList = m_cookieTree.Find( strDomain );

	if ( (unsigned long)cookie.GetExpiresValue().GetTime() != 0 )
	{
		m_boolSaved = false;
	}

	if ( ppList && *ppList )
	{	// We have a cookie list for this domain, Add, replace or delete based on cookie 
		// values.
		ChCookieList* pList = *ppList;	  

		bool		  boolDelete = false;

		if ( cookie.GetExpiresValue() != ChTime(ChTime_t(0)) && cookie.GetExpiresValue() < ChTime::GetCurrentTime() )
		{ // delete the cookie if present
			boolDelete = true;
		}
		bool boolAdd = true;
		ChPosition pos = pList->GetHeadPosition();
		while ( pos )
		{
			ChCookie oldCookie = pList->Get( pos );

			if ( oldCookie.GetPath() == cookie.GetPath() 
						&& MatchNames( oldCookie.GetNameValue(), cookie.GetNameValue() ) )
			{  // does the names match 

				if ( boolDelete )
				{
					pList->Remove( pos );
					boolAdd = false;

					if ( pList->GetCount() == 0 )
					{  // No more cokkies for this domain
						m_cookieTree.Delete( strDomain );
						delete pList;

					}
				}
				else
				{
					pList->Set( pos, cookie );
					boolAdd = false;

				}
				pos = 0;	
			}
			else
			{
				pList->GetNext( pos ); 
			}
			
		}

		if ( boolAdd && !boolDelete )
		{
			pList->AddTail( cookie );
		}
	}
	else
	{
		ChCookieList* pList = new ChCookieList();
		ASSERT( pList );
		pList->AddHead( cookie );

		m_cookieTree.Insert( strDomain, pList );

	}
	
}

/*----------------------------------------------------------------------------
	ChHttpCookie::MatchNames 
----------------------------------------------------------------------------*/

bool ChHttpCookie::MatchNames( const ChString& strName1,  const ChString& strName2 )
{
	if ( strName1.IsEmpty() || strName2.IsEmpty() )
	{
		return false;
	}
	ChString strChunk = strName1;

	int    iIndex = strChunk.Find( TEXT( ';' ) );

	while ( iIndex != -1 )
	{
		ChString strTemp = strChunk.Left( iIndex );
		// Get the name
		strTemp = strTemp.Left( strChunk.Find( TEXT( '=' ) )); 

		int iIndex2;
		if (  (iIndex2 = strName2.Find(strTemp )) == -1 )
		{
			return false;
		}
		else
		{ // check if it is a exact match
			bool boolMatch = true;
	
			if ( iIndex2 )
			{
				boolMatch = (strName2[iIndex2] == TEXT( ' ' ));	
			}
			if ( (iIndex2 + strTemp.GetLength() ) < strName2.GetLength() )
			{
				boolMatch = (strName2[iIndex2 + strTemp.GetLength() ] == TEXT( '=' ));	
			}
			else
			{
			 	boolMatch = false;
			}
			if ( !boolMatch )
			{
				return false;
			}
		}
		// Go to the next name
		strChunk = strChunk.Right( strChunk.GetLength() - iIndex - 1 );
		iIndex = strChunk.Find( TEXT( ';' ) );

	}

	return true;

}
/*----------------------------------------------------------------------------
	ChHttpCookie::IsValidDomain 
----------------------------------------------------------------------------*/

bool ChHttpCookie::IsValidDomain( const ChString& strDomain )
{
	int iNumDots = 0;
	LPCSTR pstrDomain = strDomain;

	while ( *pstrDomain )
	{
		if ( *pstrDomain == TEXT( '.' ) )
		{
			iNumDots++;
		}
		pstrDomain++;
	}

	if ( iNumDots < 2 )
	{
		return false;
	}
	else if ( iNumDots == 2 ) 
	{ // see if it is a standard domain
	  // "COM", "EDU", "NET", "ORG", "GOV", "MIL", and "INT". 

	  if ( strDomain.Find( ".com" ) != -1 ||
	  	   strDomain.Find( ".net" ) != -1 ||
	  	   strDomain.Find( ".edu" ) != -1 ||
	  	   strDomain.Find( ".org" ) != -1 ||
	  	   strDomain.Find( ".gov" ) != -1 ||
	  	   strDomain.Find( ".mil" ) != -1 || 
	  	   strDomain.Find( ".int" ) != -1 )
	  {
	   		return true;
	  }

	}
	else
	{
		return true;
	}

	return false;
}

/*----------------------------------------------------------------------------
	ChHttpCookie::DoPathMatch 
----------------------------------------------------------------------------*/

bool ChHttpCookie::DoPathMatch( ChCookieList* pList, const char* pstrPath, ChString& strCookie  )
{
	ChPosition pos = pList->GetHeadPosition();
	while ( pos )
	{
		ChCookie cookie = pList->Get( pos );
		LPCSTR  pstrCookiePath = cookie.GetPath();

		// do the match
		int iIndex = 0;
		while( pstrPath[iIndex] && pstrCookiePath[iIndex] &&
					pstrCookiePath[iIndex] == pstrPath[iIndex] )
		{
			iIndex++;
		}

		if ( pstrCookiePath[iIndex] == 0 )
		{
		 	strCookie += cookie.GetNameValue();
		}
		// Get the next path to match
		pList->GetNext( pos );
	}

   	return !strCookie.IsEmpty();
}

/*----------------------------------------------------------------------------
	ChHttpCookie::MakeCookie 
----------------------------------------------------------------------------*/

bool ChHttpCookie::MakeCookie( ChString& strBuffer, ChString& strDomain, ChCookie& cookie )
{
	if ( strBuffer.GetLength() == 0 || 
			( strBuffer.GetLength() && strBuffer[0] == TEXT( '#' ) )  )
	{
		return false;
	}

	// Domain
	int iIndex = strBuffer.Find( COOKIE_DELIMITOR );
	
	if ( iIndex == -1 )
	{
		return false;
	}
	strDomain = strBuffer.Left( iIndex );
	strBuffer = strBuffer.Right( strBuffer.GetLength() - iIndex - 1 );

	// Path
	iIndex = strBuffer.Find( COOKIE_DELIMITOR );
	
	if ( iIndex == -1 )
	{
		return false;
	}

	ChString strTemp;
	strTemp = strBuffer.Left( iIndex );
	strBuffer = strBuffer.Right( strBuffer.GetLength() - iIndex - 1 );

	cookie.SetPath( strTemp );

	// Secure
	iIndex = strBuffer.Find( COOKIE_DELIMITOR );
	
	if ( iIndex == -1 )
	{
		return false;
	}

	strTemp = strBuffer.Left( iIndex );
	strBuffer = strBuffer.Right( strBuffer.GetLength() - iIndex - 1 );
	cookie.SetSecure( strTemp == TEXT( "FALSE" ) ? false : true );

	// Expires
	iIndex = strBuffer.Find( COOKIE_DELIMITOR );
	
	if ( iIndex == -1 )
	{
		return false;
	}

	strTemp = strBuffer.Left( iIndex );
	strBuffer = strBuffer.Right( strBuffer.GetLength() - iIndex - 1 );

	ChTime timeExpires( ((ChTime_t)atol( strTemp )) );
	cookie.SetExpiresValue( timeExpires );

	// Name value
	strBuffer.TrimLeft();
	strBuffer.TrimRight();
	cookie.SetNameValue( strBuffer );

   return true;

}

int ChHttpCookie::ReadLine( char*& pstrBuffer, int& iCount, int iNext,  ChString& strLine, std::fstream* pstreamIn )
{
	int iStop = iNext;

	strLine.Empty();

	// skip all new lines
	while( iStop < iCount && 
			( pstrBuffer[iStop] == TEXT( '\r' ) || pstrBuffer[iStop] == TEXT( '\n' ) ) )
	{
		iStop++;
	}

	iNext = iStop;


	while ( iCount )
	{
		while( iStop < iCount && 
				( pstrBuffer[iStop] != TEXT( '\r' ) && pstrBuffer[iStop] != TEXT( '\n' ) ) )
		{
			iStop++;
		}

		if ( pstrBuffer[iStop] == TEXT( '\r' ) || pstrBuffer[iStop] == TEXT( '\n' ) )
		{
			pstrBuffer[iStop] = 0;

			iStop++;

			strLine += &pstrBuffer[iNext];

			return iStop;
		}
		else
		{
			pstrBuffer[iCount] = 0;

			strLine += &pstrBuffer[iNext];
			iNext = iStop = 0;
			iCount = pstreamIn->read( pstrBuffer, 4093 ).gcount();
		}
	}

	return 0;

}
