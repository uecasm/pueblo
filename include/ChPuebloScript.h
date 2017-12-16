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

#if !defined( CH_PUEBLOSCRIPT_H_ )

#define CH_PUEBLOSCRIPT_H_ 

#include <ChArgList.h>


class CH_EXPORT_CLASS ChPuebloScript
{
	public :
		enum 	tagScriptCommands { cmdMessage };

		ChPuebloScript();
		virtual ~ChPuebloScript();

		bool	ProcessScript( const char* pstrFile );

		virtual bool	ExecuteCommand( int iCommand,  ChArgumentList& attrList,
										   ChArgumentList& argList ) = 0;

	private :
		enum 	tagConst { tagBufferSize = 1024};
		enum 	tagTokens { typeUnknown = 0, typeScript, typeMessage, typeParam };

		void 	ParseScript( const char* pstrBuffer, chint32 lCount );

		void 	ProcessToken( const char* pstrToken, chint32& lStart, chint32 lCount	);

		void 	ProcessArguments( chint32 lStart, bool boolAttr );
		void 	AddNameValue( bool boolAttr, ChString& strAttr, ChString& strVal );
		int 	GetType( const ChString& strToken );

	private :
		char*				m_pstrTokenBuffer;		// internal tag buffer
		int					m_iTokenStart;			// Start of argument buffer
		int					m_iTokenSize;				// sizeof tag
		ChArgumentList	 	m_argList;
		ChArgumentList 		m_attrList;
};

#endif	// !defined( CH_PUEBLOSCRIPT_H_ )
