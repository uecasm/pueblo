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

		 Ultra Enterprises:  Gavin Lambert

		      Added default port numbers for each URL scheme.

------------------------------------------------------------------------------

	This file contains the definition of the ChHTTPInfo class, used to
	manage a connection for downloading modules and data from the server.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHURLMAP_H )
#define _CHURLMAP_H

#if defined( CH_MSW ) && defined( CH_ARCH_32 )
											/* Disable warnings about non-
												exported classes for MSW */
	#pragma warning( disable: 4275 )
	#pragma warning( disable: 4251 )

#endif	// defined( CH_MSW )

#if defined( CH_UNIX )

	#include <ChTypes.h>

#endif // CH_UNIX


#define HTTP_DEFAULT_SOCKET		80


/*----------------------------------------------------------------------------
	ChURLParts class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChURLParts
{
	public :
		enum tagSchemes {	typeHTTP = 0, typeFile, typeFTP, typeGopher, typeMailTo, 
				typeNews, typeNNTP, typeTelnet, typeWAIS, typeProspero, typeURL, maxSchemes };

		ChURLParts() : m_iScheme( typeURL ), m_pstrParts(0 ), m_pScheme(0), m_pHostName(0 ),
					   m_pAbsPath(0 ),	m_pRelPath(0), m_pAnchor(0)
		{
			m_iSocket = m_SchemeDefaultPort[typeHTTP];
		}
		 
		virtual ~ChURLParts()
		{
			if ( m_pstrParts )
			{
			 	delete [] m_pstrParts;
			}
		}
		// methods
	
		bool GetURLParts( const ChString& strURL, const char* pstrDefURL = 0 );


		
		//attributes
		int GetScheme( )		  			{ return m_iScheme; }
		virtual const ChString& GetURL();
		virtual const char*	GetHostName();
		virtual const char* GetAbsPath();
		virtual const char* GetRelPath();
		virtual int			GetPortNumber();


		// methods
		static bool MapHostFileToURL( const char* pstrLocal, ChString& strURL );
		static bool MapURLToHostFile( const char* pstrURL, ChString& strHostFile );
		static void EscapeSpecialChars( ChString& strData );
		static int GetSchemeByName( const char* pScheme );
	private :
		int	   m_iScheme;
		char * m_pstrParts;
		char * m_pScheme;		
		char * m_pHostName;
		char * m_pAbsPath;
		char * m_pRelPath;
		char * m_pAnchor;
		int	   m_iSocket;
		ChString m_strURLRequest;
		static const char * m_pstrScheme[maxSchemes];
		static const int m_SchemeDefaultPort[maxSchemes];
};

// $Log$
// Revision 1.1.1.1  2003/02/03 18:56:02  uecasm
// Import of source tree as at version 2.53 release.
//

#endif	// !defined( _CHURLMAP_H )
