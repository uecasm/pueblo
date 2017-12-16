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


----------------------------------------------------------------------------*/

#include "headers.h"
#include <ChUrlMap.h>
#include <ChHTTP.h>

#ifdef CH_UNIX
#include <stdlib.h>
#endif

#include <MemDebug.h>

const char * ChURLParts::m_pstrScheme[] =
		{
			   "http",                 //   Hypertext Transfer Protocol
			   "file",                 //   Host-specific file names
			   "ftp",                  //	File Transfer protocol
			   "gopher",               //   The Gopher protocol
			   "mailto",               //   Electronic mail address
			   "news",                 //   USENET news
			   "nntp",                 //   USENET news using NNTP access
			   "telnet",               //   Reference to interactive sessions
			   "wais",                 //   Wide Area Information Servers
			   "prospero",             //   Prospero Directory Service
			   "url",				   //   
		};

const int ChURLParts::m_SchemeDefaultPort[] =
{
				80,											// http://
				80,											// file:// (no port used, so default is irrelevant)
				21,											// ftp://
				70,											// gopher://
				25,											// mailto: (SMTP)
				119,										// news://
				119,										// nntp://
				23,											// telnet://
				80,											// wais:// (don't actually know)
				80,											// prospero:// (dunno either)
				80,											// unrecognised scheme
};


/*----------------------------------------------------------------------------
	ChURLParts class  implementation
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPInfo::GetURLParts( )

------------------------------------------------------------------------------

Get the different parts of a given URL.

----------------------------------------------------------------------------*/

bool ChURLParts::GetURLParts( const ChString& strURL, const char* pstrDefURL )
{
    char*	after_access;
    char*	p;
	char*	pstrHostName = NULL;

	#ifdef CH_MSW

    int		iLength = strURL.GetLength();


	#else

	int		iLength = strURL.length();
	#endif

	int 	iExtra  =  (pstrDefURL) ? lstrlen( pstrDefURL )  : 0;

	// if we have a old URL part delete the parts
	if ( m_pstrParts )
	{
		delete [] m_pstrParts;
	}

									// +2 in case  we have pstrDefURL
	m_pstrParts = new char[iLength + iExtra + 2 ];
	ASSERT( m_pstrParts );

	lstrcpy( m_pstrParts, strURL );

	if ( pstrDefURL )
	{// copy the default host name, this will be used if there is no
	  // host name specified in strURL
		pstrHostName = m_pstrParts + (lstrlen( m_pstrParts ) + 1 );
		lstrcpy( pstrHostName, pstrDefURL );
	}

    m_pScheme 	= 0;
    m_pHostName = 0;
    m_pAbsPath 	= 0;
    m_pRelPath 	= 0;
    m_pAnchor 	= 0;
    m_iSocket 	= m_SchemeDefaultPort[typeHTTP]; //overide this if there is
										   // is a socket # in the URL
    after_access = m_pstrParts;

    for( p=m_pstrParts; *p; p++)
    {
		if (*p==':')
		{
			*p = 0;
			m_pScheme = after_access; /* Scheme has been specified */

			after_access = p+1;

			#if defined( CH_MSW )
				#if defined( CH_ARCH_16 )
					AnsiLower( m_pScheme );
				#else
					CharLower( m_pScheme );
				#endif
			#else
				for (char *cp = m_pScheme; cp && *cp; cp++)
					*cp = tolower(*cp);
//				strlwr( m_pScheme );
			#endif

			m_iScheme = GetSchemeByName( m_pScheme );

			// use default port number for specified scheme
			m_iSocket = m_SchemeDefaultPort[m_iScheme];

			if ( m_iScheme == typeURL )
			{
			    m_pScheme = 0;
			}
			else if ( m_iScheme == typeMailTo )
			{  // mailto is a special case, handle it differently
				m_pHostName = after_access;
				m_strURLRequest = strURL;
				return true;
			}
			else
				break;
		}
		if (*p=='/')
			break;		/* Access has not been specified */
		if (*p=='#')
			break;
    }

    for( p=m_pstrParts+iLength-1; p>=m_pstrParts; p-- )
    {
		if (*p =='#')
		{
		    m_pAnchor=p+1;
		    *p=0;				/* terminate the rest */
		}
    }
    p = after_access;
    if (*p=='/')
    {
		if (p[1]=='/')
		{
		    m_pHostName = p+2;						/* pHostName has been specified 	*/
		    *p=0;									/* Terminate pScheme 		*/
		    p=strchr(m_pHostName,'/');				/* look for end of pHostName pName if any */
		    if(p)
		    {
		        *p=0;								/* Terminate pHostName */
		        m_pAbsPath = p+1;					/* Root has been found */
		    }
			// do we have a port number in the host name
			p=strchr( m_pHostName, TEXT(':') );		/* look for port number */
			if ( p )
			{
				*p = 0;								// remove the port delimiter
				m_iSocket = (int)atol( p + 1 );		// get the port number
			}


		}
		else
		{
	    	m_pAbsPath = p+1;				/* Root found but no pHostName */
		}
    }
    else
    {
        m_pRelPath = (*after_access) ? after_access : 0;	/* zero for "" */
    }

	// check if we have a host name
	if ( NULL == m_pHostName && pstrHostName )
	{
	    after_access = pstrHostName;

	    for( p=pstrHostName; *p; p++)
	    {
			if (*p==':')
			{
				*p = 0;
				m_pScheme = after_access; /* Scheme has been specified */
				after_access = p+1;


				#if defined( CH_MSW )
					#if defined( CH_ARCH_16 )
						AnsiLower( m_pScheme );
					#else
						CharLower( m_pScheme );
					#endif
				#else
					for (char *cp = m_pScheme; cp && *cp; cp++)
						*cp = tolower(*cp);
//					strlwr( m_pScheme );
				#endif

				m_iScheme = GetSchemeByName( m_pScheme );

				if ( m_iScheme == typeURL )
				{
				    m_pScheme = 0;
				}
				else
					break;
			}
			if (*p=='/')
				break;		/* Access has not been specified */
			if (*p=='#')
				break;
	    }

	    p = after_access;

	    if (*p=='/' &&  p[1]=='/' )
	    {
		    m_pHostName = p+2;						/* pHostName has been specified 	*/
		    *p=0;									/* Terminate pScheme 		*/
		    p=strchr( m_pHostName,'/');				/* look for end of pHostName pName if any */
		    if(p)
		    {
		        *p=0;								/* Terminate pHostName */
				if ( m_iScheme == typeFile )
				{  // for file make the volume the host name
					char *pTmp = p + 1;
				    pTmp = strchr( p + 1,'|');	 /* look for end of volume name if any */

					if ( pTmp && *(pTmp + 1) == '/'  )
					{
						*p = '/';
						m_pHostName = p;
						*(pTmp + 1) = 0;
						p = pTmp + 1;
					}
				}
		    }

			if ( NULL == m_pAbsPath  )
			{
				if (  m_pRelPath )
				{
					if ( p )
					{
	    				m_pAbsPath = p+1;		/* use the abs path of defURL */
						// remove the file name if any
		    			p=strrchr( m_pAbsPath,'/');		/* look for filename  */
						if ( p )
						{
							*(p + 1)= 0;
						}
					}

					if ( NULL == p )
					{ // relative path is the absolute path
	    				m_pAbsPath = m_pRelPath;
						m_pRelPath = NULL;
					}
					else
					{ // remove any relative path info
						while ( m_pRelPath[0] == TEXT( '.' ) )
						{
							
							int i = 0;
							while ( m_pRelPath[i] && m_pRelPath[i] == TEXT( '.' ) )
							{
								i++;
							}

							if ( i == 2 )
							{
								ChMemCopy( m_pRelPath, &m_pRelPath[i + 1], lstrlen( m_pRelPath ) - 2 );
								// Remove one level from the abspath
								int iEnd = lstrlen( m_pAbsPath );
								if ( iEnd  && m_pAbsPath[iEnd - 1] == TEXT( '/' ) )
								{
									m_pAbsPath[iEnd - 1 ] = 0;
								}
				    			char* pTmp =strrchr( m_pAbsPath,'/');		/*remove one level  */
								if ( pTmp )
								{
									*(pTmp + 1)= 0;
								}
								else
								{
									m_pAbsPath[0] = 0;
								}
							}
							else
							{
								ChMemCopy( m_pRelPath, &m_pRelPath[i + 1 ], lstrlen( m_pRelPath ) - 1 );
							}
						}
					}
				}
			    else
			    {   // if there is no abs path then there should be a relative path
					return ( false );
			    }

			}

			// do we have a port number in the host name
			p=strchr( m_pHostName, TEXT(':') );	/* look for port number */
			if ( p )
			{
				*p = 0;										// remove the port delimiter
				m_iSocket = (int)atol( p + 1 );				// get the port number
			}

	    }
	    else
	    {   // defURL should have a hostname
			return ( false );
	    }

	}

	// check to see if we have a host name
	if ( 0 == m_pHostName )
	{  // no host name, this is a must to do any work.
		return false;
	}

	// Construct the URL from the parts we have
	m_strURLRequest = "";
	if ( m_pScheme )
	{
		m_strURLRequest += m_pScheme;
		m_strURLRequest += TEXT( "://" );
	}
	if ( m_pHostName )
	{
		m_strURLRequest += m_pHostName;
		if ( m_SchemeDefaultPort[m_iScheme] != m_iSocket )
		{
			char strPort[25];
			::wsprintf( strPort, ":%d", m_iSocket );
			m_strURLRequest += strPort;
		}
		m_strURLRequest += TEXT( "/" );
	}
	if ( m_pAbsPath )
	{
		m_strURLRequest += m_pAbsPath;

		if ( m_pRelPath &&
				TEXT( '/' ) != m_strURLRequest[m_strURLRequest.GetLength()-1])
		{
			m_strURLRequest += TEXT( "/" );
		}

	}

	if ( m_pRelPath )
	{
		m_strURLRequest += m_pRelPath;
	}

	if ( m_pAnchor )
	{
		m_strURLRequest += TEXT( "#" );
		m_strURLRequest += m_pAnchor;
	}

	return true;
}



void ChURLParts::EscapeSpecialChars( ChString& strData )
{
	ChString strTmp;
	int i = 0;
	while( i < strData.GetLength() )
	{
		
		switch( strData[i]  )
		{
			case '/' :
			case '?' :
			case '#' :
			case '%' :
			case '-' :
			case '.' :
			case '&' :
			case '=' :
			case '_' :
			case ':' :
			case '+' :
			{
				strTmp += strData[i];
				break;
			}
			default :
			{
				if ( !isalnum( strData[i] ))
				{
					char	strNum[10];

					strTmp += TEXT( '%' );
					wsprintf( strNum, "%x", (int)strData[i] );
					strTmp += strNum;
				}
				else
				{
					strTmp += strData[i];
				}
				break;
			}
		}

		i++;
	}	
	strData = strTmp;
}


const ChString& ChURLParts::GetURL()			
{ 
	return  m_strURLRequest; 
}


const char*	ChURLParts::GetHostName()		
{ 	// if we are using proxy server then use that as the host name
	return  m_pHostName; 
}

const char* ChURLParts::GetAbsPath()		
{ 
	return  m_pAbsPath; 
}

const char* ChURLParts::GetRelPath()		
{ 
	return  m_pRelPath; 
}

int	ChURLParts::GetPortNumber()		
{ 
	return  m_iSocket; 
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChHTTPInfo::GetSchemeByName( )

------------------------------------------------------------------------------

Get the different parts of a given URL.

----------------------------------------------------------------------------*/

int ChURLParts::GetSchemeByName( const char* pScheme )
{
	for ( int i = 0; i < maxSchemes; i++ )
	{
		if ( lstrcmp( pScheme, m_pstrScheme[i] ) == 0 )
		{
			return i;
		}
	}
	// set invalid to typeURL
	return typeURL;
}

bool ChURLParts::MapHostFileToURL( const char* pstrLocal, ChString& strURL )
{

   	ChString strPath( pstrLocal );

	// Set the scheme
	strURL = m_pstrScheme[typeFile];
	strURL += TEXT( ":///" );
	
	char* pstrHostFile = strPath.GetBuffer( MAX_PATH );
	ASSERT( pstrHostFile );

	#if defined( CH_MSW )
	if ( 0 == _fullpath( pstrHostFile, pstrLocal, MAX_PATH ) )
	{
		return false;
	}
	#endif

	// all host file name begin with /, if there is one already, ignore it
	if ( *pstrHostFile == TEXT( '\\' ) ||  *pstrHostFile == TEXT( '\\' ) )
	{
		++pstrHostFile;
	}

	ChString strTemp( pstrHostFile );


	strPath.ReleaseBuffer();

	// Map all special characters

	for( int i = 0; i < strTemp.GetLength(); i++ )
	{
		switch( strTemp[i] )
		{
			case TEXT( '\\' ) :
			{
				strTemp.SetAt( i, TEXT( '/' ));
				break;
			}
			case TEXT( ':' ) :
			{
				strTemp.SetAt( i, TEXT( '|' ));
				break;
			}
			default :
			{
				break;
			}
		}
	}

	strURL += strTemp;

	return true;
}

bool ChURLParts::MapURLToHostFile( const char* pstrURL, ChString& strHostFile )
{
	ChString 	strURL( pstrURL );

	strHostFile.Empty();

	if ( strURL.Find( m_pstrScheme[typeFile] ) != 0 )
	{  // unknown scheme, cannot map to locl host file
		return false;
	}

    const 	char* after_access = pstrURL;

    for( const char*p=pstrURL; *p; p++ )
    {
		if ( *p==':' )
		{
			after_access = p+1;
			break;
		}
		if (*p=='/')
			break;		/* Access has not been specified */
		if (*p=='#')
			break;
    }

    p = after_access;

    if (*p=='/')
    {
		if (p[1]=='/')
		{
			if ( p[2] == '/' ) 
			{
				p += 3;
			}
			else
			{
				p += 2;
			} 

			for( ; *p && *p != TEXT( '#' ); p++ )
			{
				switch( *p )
				{
					case TEXT( '|' ) :
					{
						strHostFile += TEXT( ':' );
						break;
					}
					#if defined( CH_MSW )
					case TEXT( '/' ) :
					{
						strHostFile += TEXT( '\\' );
						break;
					}
					#endif
					default :
					{
						strHostFile += *p;
						break;
					}
				}
			}
		}
		else
		{
			return false;
		}
    }
	return true;
}
