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

// $Header$

#if (!defined( _CHHTTPCOOKIE_H ))
#define _CHHTTPCOOKIE_H

#include <fstream>
#include <ChTime.h>
#include <ChSplay.h>
#include <ChList.h>


class ChCookie
{
	public :
		ChCookie( ) :
						m_boolSecure( false )
					{
					}
		ChCookie( const ChTime& timeExpires, bool boolSecure, const ChString& strNameValue,
						const ChString& strPath ) :
						m_timeExpires( timeExpires ),
						m_boolSecure( boolSecure ),
						m_strNameValue(  strNameValue ),
						m_strPath( strPath )
					{
					}

		ChCookie( const ChCookie& cookie )
					{
						m_timeExpires 	= cookie.m_timeExpires;
						m_boolSecure  	= cookie.m_boolSecure;
						m_strNameValue	= cookie.m_strNameValue;
						m_strPath		= cookie.m_strPath;
					}
		virtual ~ChCookie() {}
		const ChCookie& operator=( const ChCookie& cookie )
					{
						m_timeExpires 	= cookie.m_timeExpires;
						m_boolSecure  	= cookie.m_boolSecure;
						m_strNameValue	= cookie.m_strNameValue;
						m_strPath		= cookie.m_strPath;	 
						return *this;
					}
		const ChString& GetPath() const 			{ return  m_strPath; }
		const ChString& GetNameValue() const 		{ return  m_strNameValue; }
		const ChTime& GetExpiresValue() const 	{ return  m_timeExpires; }
		bool  IsExpired( const ChTime& time )	{ return  ( time > m_timeExpires); }
		bool  IsSecure(	)						{ return  m_boolSecure; }

		void  SetPath( const ChString& strPath) 						{ m_strPath = strPath; }
		void  SetNameValue( const ChString& strNameValue ) 	 		{ m_strNameValue = strNameValue; }
		void  SetExpiresValue( const ChTime& timeExpires )  		{ m_timeExpires =  timeExpires ; }
		void  SetSecure( bool boolSecure	)						{ m_boolSecure = boolSecure; }


	private :
		ChTime		m_timeExpires;
		bool		m_boolSecure;
		ChString		m_strNameValue;
		ChString		m_strPath;

	
};

// List of cookie info
typedef ChList<ChCookie>	ChCookieList;

typedef ChSplay<ChString, ChCookieList*>	ChCookieTree;   

//class fstream;        

class ChHttpCookie
{
	public :
		ChHttpCookie();
		~ChHttpCookie();
		
		// Attributes

		bool ReadCookieFile( const char* pstrFile = 0 );

		bool WriteCookieFile( const char* pstrFile = 0 );

		bool GetCookie( const ChString& strURL, ChString& strCookie, bool& boolSecure );

		bool SetCookie( const ChString& strURL, const ChString& strCookie );  

		inline bool IsSaved()			{ return  m_boolSaved; }
	private :
		bool MatchNames( const ChString& strName1,  const ChString& strName2 );
		void AddCookie( const ChString& strDomain, ChCookie& cookie );
		bool MakeCookie( ChString& strBuffer, ChString& strDomain, ChCookie& cookie );
		bool IsValidDomain( const ChString& strDomain );
		bool DoPathMatch( ChCookieList *pList, const char*  strPath, ChString& strCookie  );
		int  ReadLine( char*& pstrBuffer, int& iCount, int iNext,  ChString& strLine, std::fstream* pstreamIn );






	private :		
		ChString 			m_strCookieFile; 	
		ChCookieTree	m_cookieTree;
		bool			m_boolSaved;
};

#endif // (!defined( _CHHTTPCOOKIE_H ))

// $Log$
