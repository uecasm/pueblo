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

	This file consists of the interface for the following classes:
	
		ChArgumentList


----------------------------------------------------------------------------*/

// $Header$

#if (!defined( _CHARGUMENTLIST_H ))
#define _CHARGUMENTLIST_H

#include <ChList.h>  

#define CMD_LINE	"cmdline"


typedef ChList<ChString>	ChArgArray; 


class CH_EXPORT_CLASS ChArgumentList
{
	public :
		ChArgumentList()				{}	
		ChArgumentList( const ChArgumentList& argCopy );	
		virtual ~ChArgumentList()		{}
		int GetArgCount() const			{ return  m_argName.GetCount(); }
		bool GetArg( int iIndex, ChString& strName, ChString& strValue ) const;
		bool FindArg( const char* pstrName, ChString& strValue );

		void AddArg( const ChString& strName, 	const ChString&  strValue );
		void Empty();
	private :
		ChArgArray			m_argName;
		ChArgArray			m_argValue;
		
};

// $Log$

#endif // (!defined( _CHARGUMENTLIST_H ))
