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

	This file contains the interface for the ChModuleMgr class, which
	manages module information as well as loading and unloading modules.

----------------------------------------------------------------------------*/

#if !defined( _CHRUNTIMEINFO_H )
#define _CHRUNTIMEINFO_H

#include <ChModule.h>
#include <ChSplay.h>

#define CH_LOCAL_MODULE_ID_START 	0xFFFFF000
#define CH_LOCAL_MODULE_ID_END		0xFFFFFFFF

class  ChModuleRunInfo;

typedef ChModuleRunInfo*  pChModuleRunInfo;


/*----------------------------------------------------------------------------
	ChModuleInfo class
----------------------------------------------------------------------------*/

class ChModuleRunInfo
{
	public:
		#if defined( CH_MSW )

		ChModuleRunInfo( const ChString& strModule,  HINSTANCE hLibrary );

		#else	// defined( CH_MSW )

		ChModuleRunInfo( const ChString& strModule );

		#endif	// defined( CH_MSW )

		~ChModuleRunInfo();

		inline ChDispatcher* GetDispatcher() const 	{ return m_pDispatcher; }
		inline ChMainHandler GetMainHandler() const { return m_mainHandler; }
		inline ChMainInfo* GetMainInfo() const 		{ return m_pMainInfo; }
		inline const ChString& GetName() const 		{ return m_strName; }

		inline void SetDispatcher( ChDispatcher* pDispatcher )
						{
							m_pDispatcher = pDispatcher;
						}

		inline void Set( ChMainHandler mainHandler, ChMainInfo* pMainInfo )
						{
							m_mainHandler = mainHandler;
							m_pMainInfo = pMainInfo;
						}

		inline int GetUseCount() const { return m_iUseCount; }
		inline void Use() { m_iUseCount++; }
		inline void Release() { m_iUseCount--; }

		bool Unload( const ChModuleID idModule );

	private:
		ChString			m_strName;
		ChMainInfo*		m_pMainInfo;
		ChMainHandler	m_mainHandler;
		ChDispatcher*	m_pDispatcher;
		int				m_iUseCount;

		#if defined( CH_MSW )

		HINSTANCE		m_hLibrary;

		#endif	// defined( CH_MSW )
};


/*----------------------------------------------------------------------------
	Destructor helper
----------------------------------------------------------------------------*/

static void ChPtrSplayDestruct( ChModuleRunInfo* pItem )
{
	delete pItem;
}  

#endif	// !defined( _CHRUNTIMEINFO_H )
