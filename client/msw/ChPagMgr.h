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

	This file contains the interface for the ChPageManager class,
	which handles management of module preference and about pages.

----------------------------------------------------------------------------*/

#if !defined( _CHPAGMGR_H )
#define _CHPAGMGR_H

#include <ChMsg.h>
#include <ChMsgTyp.h>
#include <ChList.h>



class ChCore;

/*----------------------------------------------------------------------------
	ChPageManager class
----------------------------------------------------------------------------*/

class ChModulePagesInfo;

class ChPageManager
{
	public:
		ChPageManager( ChCore* pCore, ChPageType type );
		~ChPageManager();

		void AddModulePages( CPropertySheet* pSheet );
		void GetPageData();
		void ReleaseModulePages();

	protected:
		void AddModulePages( CPropertySheet* pSheet,
								const ChModuleID& idModule,
								int iPageCount, chparam* pPages );
		void SaveModulePages( const ChModuleID& idModule,
								int iPageCount, chparam* pPages );
		void GetPageData( ChModulePagesInfo* pPagesInfo );
		void ReleaseModulePages( ChModulePagesInfo* pPagesInfo );

	protected:
		ChPageType	m_type;
		ChParamList	m_pageInfoList;
		ChCore* 	m_pCore;


};

#endif	// !defined( _CHPAGMGR_H )
