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

	This file contains the implementation of Pueblo module manager core.  This
	file is only used on the client.

----------------------------------------------------------------------------*/

#include "headers.h"

#include <iostream>
#include <fstream>

#include <ChPuebloScript.h>

#include <MemDebug.h>

#define CHAR_SPACE			TEXT( ' ' )
#define CHAR_DBL_QUOTE		TEXT( '"' )
#define CHAR_SGL_QUOTE		TEXT( '"' )

#define IS_WHITE_SPACE( c )	  ( isspace( (c) ) || ((c) == 0x0A ) )


CH_GLOBAL_VAR  const char* apstrScriptTokens[] =
						{
							"unknown",
							"script",
							"ch_message",
							"param",
						};



ChPuebloScript::ChPuebloScript() :
					m_iTokenStart( 0 )
{

	m_pstrTokenBuffer = new char[tagBufferSize];
	ASSERT( m_pstrTokenBuffer );
	m_iTokenSize = 0;

}
ChPuebloScript::~ChPuebloScript()
{
	delete [] m_pstrTokenBuffer;
}

int ChPuebloScript::GetType( const ChString& strToken )
{
	for( int i = 1; i < sizeof( apstrScriptTokens ) / sizeof( char* ); i++ )
	{
		if ( strToken.CompareNoCase( apstrScriptTokens[i] )  == 0  )
		{
			return i;
		}
		
	}
	return typeUnknown;
}

bool ChPuebloScript::ProcessScript( const char* pstrFile )
{

	//#ifdef CH_MSW
//		fstream *pFile = ::new fstream( pstrFile, ios::in | ios::nocreate , filebuf::sh_read );
		std::fstream *pFile = ::new std::fstream( pstrFile, std::ios::in );
	//#else
	//	fstream *pFile = ::new fstream( pstrFile, ios::in | ios::nocreate );
	//#endif

	ASSERT( pFile );

	if ( !pFile->is_open() )
	{
		::delete pFile;
		return false;
	}

	// UE; removed because BC doesn't like it, and files default to text mode anyway
//	#if defined( CH_MSW )
//	{										// set the file read to text mode
//		pFile->setmode( filebuf::text );
//	}
//	#endif

	char *pstrBuffer = new char[ 4092 ];
	ASSERT( pstrBuffer );

	// parse the HTML text file
	while( ( pFile->read( pstrBuffer, 4092 ).gcount()) )
	{
		ParseScript( pstrBuffer, pFile->gcount() );
	}

	pFile->close();

	::delete pFile;
	delete []pstrBuffer;

	return true;
}

void ChPuebloScript::ParseScript( const char* pstrText, chint32 lLength )
{
	chint32 lStart 		= 0;

	if ( lLength == -1 )
	{
		lLength = lstrlen( pstrText );
	}

	if ( m_iTokenSize )
	{ // we have a incomplete tag
		ProcessToken( pstrText, lStart, lLength );
	}


	// process all the characters in the buffer
	while ( lStart < lLength )
	{
		switch( pstrText[lStart] )
		{
			case TEXT( '<' ) :
			{	// New command ?
				ProcessToken(  pstrText, lStart, lLength );
				break;
			}
			default :
			{   // skip till I see the next command
				lStart++;
				break;
			}
		}
	}

	return;
}

void ChPuebloScript::ProcessToken( const char* pstrBuffer, chint32& lStart, chint32 lCount	)
{
	bool boolEnd = false;

	int  iType = typeUnknown;

	// get the HTML tag to our local buffer
	while( lStart < lCount &&
				pstrBuffer[lStart] != TEXT( '>' ) && m_iTokenSize < tagBufferSize )
	{
		m_pstrTokenBuffer[m_iTokenSize++] = pstrBuffer[lStart++];
	}

	// is the tag terminated ?
	if ( lStart >= lCount || pstrBuffer[lStart] != TEXT( '>' ) )
	{ // partial tag
		//save this and return

		if ( m_iTokenSize >= tagBufferSize )
		{ // token is too long, this should be a bogus tag
		  // skip it completly
			while( lStart < lCount &&
					pstrBuffer[lStart++] != TEXT( '>' ) )

			// remove all new lines characters
			while( lStart < lCount &&  IS_WHITE_SPACE( pstrBuffer[lStart] ) )
			{
				++lStart;
			}
		}

		return;

	}
	// the '>' char
	m_pstrTokenBuffer[m_iTokenSize++] = pstrBuffer[lStart++];


	ASSERT( m_pstrTokenBuffer[0] == TEXT( '<' ) );

 	// Process the tag
	m_iTokenStart = 1;	//skip '<'

	if ( m_pstrTokenBuffer[m_iTokenStart] == TEXT( '/' ))
	{	// termination tag
		boolEnd = true;
		m_iTokenStart++;
	}

	// special processing for comment
	if ( m_pstrTokenBuffer[m_iTokenStart] == TEXT( '!' ))
	{
		// remove all new lines characters
		while( lStart < lCount &&  (  pstrBuffer[lStart] == TEXT( '\r' )
									|| pstrBuffer[lStart] == TEXT( '\n' )) )
		{
			++lStart;
		}
		m_iTokenSize = 0;
		return;
	}

	// get the tag name
	ChString strToken = TEXT( "" );

	while( m_iTokenStart < lCount  &&  m_pstrTokenBuffer[m_iTokenStart] != CHAR_SPACE
							&&  m_pstrTokenBuffer[m_iTokenStart] != TEXT( '\r' )
							&&  m_pstrTokenBuffer[m_iTokenStart] != TEXT( '\n' )
							&&  m_pstrTokenBuffer[m_iTokenStart] != TEXT( '>' ) )
	{
		strToken += m_pstrTokenBuffer[m_iTokenStart++];
	}

	strToken.MakeLower();  
	strToken.TrimLeft();  
	strToken.TrimRight();  

	iType = GetType( strToken );

	if ( iType == typeUnknown )
	{
		m_iTokenSize = 0;
		return;
	}

	switch ( iType )
	{
		case typeScript :
		{
			// clear all attrs and arguments
			m_attrList.Empty();
			m_argList.Empty();
			break;
		}
		case typeMessage :
		{
			if ( !boolEnd )
			{
				ProcessArguments( m_iTokenStart, 	true );
			}
			else
			{  // execute the command
				ExecuteCommand( cmdMessage, m_attrList, m_argList );
			}
			break;
		}
		case typeParam :
		{
			ProcessArguments( m_iTokenStart, 	false );
			break;
		}
	}

	m_iTokenSize = 0;
	return;
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChPuebloScript::ProcessArguments

------------------------------------------------------------------------------
This method translates all arguments and adds it to attrList or argList based on
boolAttr argument.
----------------------------------------------------------------------------*/
void ChPuebloScript::ProcessArguments( chint32 lStart, bool boolAttr )
{

	if ( m_pstrTokenBuffer[lStart] == TEXT( '>' ))
	{
		return;
	}

	// remove the tag
	while( lStart < m_iTokenSize && m_pstrTokenBuffer[lStart] != TEXT( '>' )
					&& !IS_WHITE_SPACE( m_pstrTokenBuffer[lStart] ) )
	{
		lStart++;
	}



	// Get all the attributes
	while( true )
	{
		// remove any leading white  space
		while( lStart < m_iTokenSize &&  IS_WHITE_SPACE( m_pstrTokenBuffer[lStart] ) )
		{
			++lStart;
		}
		if ( lStart >= m_iTokenSize || m_pstrTokenBuffer[lStart] == TEXT( '>' ) )
		{
			break;
		}

		// Get the argument
		ChString strAttr = TEXT( "" );
		while( lStart < m_iTokenSize  && m_pstrTokenBuffer[lStart] != TEXT( '=' )
								 && m_pstrTokenBuffer[lStart] != CHAR_SPACE
								&&  m_pstrTokenBuffer[lStart] != TEXT( '>' ) )
		{
			strAttr += m_pstrTokenBuffer[lStart++];
		}

		while( lStart < m_iTokenSize &&  IS_WHITE_SPACE( m_pstrTokenBuffer[lStart] ) )
		{
			++lStart;
		}


		if ( m_pstrTokenBuffer[lStart] == TEXT( '=' ) )
		{// we have a attribute
			// remove the equal sign
			lStart++;
			while( lStart < m_iTokenSize &&  IS_WHITE_SPACE( m_pstrTokenBuffer[lStart] ))
			{
				++lStart;
			}

			// Get the value
			ChString 	strVal = TEXT( "" );

			if ( CHAR_DBL_QUOTE == m_pstrTokenBuffer[lStart] || 
							CHAR_SGL_QUOTE == m_pstrTokenBuffer[lStart] )
			{
				char strTerm = m_pstrTokenBuffer[lStart] ;
				lStart++;
				while( lStart < m_iTokenSize  && m_pstrTokenBuffer[lStart] != strTerm
										&&  m_pstrTokenBuffer[lStart] != TEXT( '>' ) )
				{		    
					strVal += m_pstrTokenBuffer[lStart++];
				}
			}
			else
			{
				while( lStart < m_iTokenSize  && !IS_WHITE_SPACE( m_pstrTokenBuffer[lStart] )
										&&  m_pstrTokenBuffer[lStart] != TEXT( '>' ) )
				{		    
					strVal += m_pstrTokenBuffer[lStart++];
				}
			}
			lStart++;

			AddNameValue( boolAttr, strAttr, strVal );


		}
		else
		{
			// remove any leading white  space
			while( lStart < m_iTokenSize && IS_WHITE_SPACE( m_pstrTokenBuffer[lStart] ))
			{
				++lStart;
			}

			// update our style
			ChString 	strVal = TEXT( " " );		// updateattributes does not work
											 	//if strVal is empty
			AddNameValue( boolAttr, strAttr, strVal );
	

		}

	}

	return;

}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChPuebloScript::AddNameValue

------------------------------------------------------------------------------
This method translates the string value of the argument to a known quantity.
----------------------------------------------------------------------------*/
void ChPuebloScript::AddNameValue( bool boolAttr, ChString& strName, ChString& strValue )
{
	if ( strName.IsEmpty() )
	{
		return;
	}

	if ( boolAttr )
	{
		m_attrList.AddArg( strName, strValue ); // empty the user args for this tag
	}
	else
	{
		m_argList.AddArg( strName, strValue ); // empty the user args for this tag
	}
}
