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

	This file contains the inmplementationm for the following classes:
	
		ChArgumentList


----------------------------------------------------------------------------*/

#include "headers.h"
#include <ChArgList.h>

#include <MemDebug.h>

ChArgumentList::ChArgumentList( const ChArgumentList& argCopy )	
{  
	for ( int i = 0; i < argCopy.GetArgCount(); i++ )
	{ 
		ChString strName,  strValue;

		if ( argCopy.GetArg( i, strName, strValue ) )
		{
			m_argName.AddTail( strName );	
			m_argValue.AddTail( strValue );	
		}
	}
	
}

bool ChArgumentList::GetArg( int iIndex, ChString& strName, ChString& strValue ) const
{
	ChPosition pos1 = m_argName.FindIndex( iIndex );
 	ChPosition pos2 = m_argValue.FindIndex( iIndex );

	if ( pos1 && pos2 )
	{
		strName = m_argName.Get( pos1 );	
		strValue = m_argValue.Get( pos2 );	
	} 
	return pos1 && pos2;
}


bool ChArgumentList::FindArg( const char* pstrName, ChString& strValue )
{
	int iIndex = 0;
	ChPosition pos = m_argName.GetHeadPosition();

	while( pos )
	{
		ChString strName = m_argName.GetNext( pos );	

		if ( strName.CompareNoCase( pstrName ) == 0 )
		{
 			pos = m_argValue.FindIndex( iIndex );
			strValue = m_argValue.Get( pos );
			
			return true;	

		}
		iIndex++;
	}
	return false;	
}

void ChArgumentList::Empty( )
{
	m_argName.Empty( );	
	m_argValue.Empty( );	
}

void ChArgumentList::AddArg( const ChString& strName, 	const ChString&  strValue )
{
	m_argName.AddTail( strName );	
	m_argValue.AddTail( strValue );	
}
