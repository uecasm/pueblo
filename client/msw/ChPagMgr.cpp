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

	This file contains the implementation of the ChPageManager class,
	which handles management of module preference and about pages.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"
#include "ChClCore.h"

#include <ChReg.h>

#include "ChPagMgr.h"



/*----------------------------------------------------------------------------
	ChModulePagesInfo class
----------------------------------------------------------------------------*/

class ChModulePagesInfo
{
	public:
		ChModulePagesInfo( const ChModuleID& idModule, int iPageCount,
							chparam* pPages ) :
				m_idModule( idModule ),
				m_iPageCount( iPageCount ),
				m_pPages( pPages )
			{
			}

		~ChModulePagesInfo() {}

		inline const ChModuleID& GetModuleID() { return m_idModule; }
		inline int GetPageCount() { return m_iPageCount; }
		inline chparam* GetPages() { return m_pPages; }

	protected:
		ChModuleID	m_idModule;
		int			m_iPageCount;
		chparam*	m_pPages;
};


/*----------------------------------------------------------------------------
	ChPageManager class
----------------------------------------------------------------------------*/

ChPageManager::ChPageManager( ChCore* pCore, ChPageType type ) : 
						m_type( type ),
						m_pCore( pCore )
{
}


ChPageManager::~ChPageManager()
{
}


/*----------------------------------------------------------------------------
	ChPageManager public methods
----------------------------------------------------------------------------*/

void ChPageManager::AddModulePages( CPropertySheet* pSheet )
{
	int				iModuleCount = m_pCore->GetModuleCount();

	if (iModuleCount)
	{
		ChModuleID*		pModules;
		int				iMax;
		int				iLoop;

		pModules = new ChModuleID[iModuleCount];

		iMax = m_pCore->GetModuleIDs( iModuleCount, pModules );
		ASSERT( iMax == iModuleCount );

		for (iLoop = 0; iLoop < iMax; iLoop++)
		{
			ChModuleID			idModule = pModules[iLoop];
			//ChClientModule		module( idModule );
			ChGetPageCountMsg	getPageCountMsg( m_type );
			int					iPageCount;

			iPageCount =  m_pCore->DispatchMsg( idModule, getPageCountMsg );

			if (iPageCount)
			{								/* This module does have preference
												pages.  Get them from the
												module */
				chparam*	pPages;

				pPages = new chparam[iPageCount];

				if (pPages)
				{
					ChGetPagesMsg	getPagesMsg( m_type, iPageCount, pPages );
		
					m_pCore->DispatchMsg( idModule, getPagesMsg );

					AddModulePages( pSheet, idModule, iPageCount, pPages );
				}
			}
		}

		if (pModules)
		{
			delete [] pModules;
		}
	}
}


void ChPageManager::GetPageData()
{
	ChPosition		pos = m_pageInfoList.GetHeadPosition();

	while (pos)
	{
		ChModulePagesInfo*	pInfo;

		pInfo = (ChModulePagesInfo*)m_pageInfoList.Get( pos );

		if (pInfo)
		{
			GetPageData( pInfo );
		}

		m_pageInfoList.GetNext( pos );
	}
}


void ChPageManager::ReleaseModulePages()
{
	ChPosition		pos;

	while (pos = m_pageInfoList.GetHeadPosition())
	{
		ChModulePagesInfo*	pInfo;

		pInfo = (ChModulePagesInfo*)m_pageInfoList.Get( pos );

		if (pInfo)
		{
			ReleaseModulePages( pInfo );

			m_pageInfoList.Remove( pos );
			delete pInfo;
		}
	}
}


/*----------------------------------------------------------------------------
	ChPageManager protected methods
----------------------------------------------------------------------------*/

void ChPageManager::AddModulePages( CPropertySheet* pSheet,
									const ChModuleID& idModule,
									int iPageCount, chparam* pPages )
{
	int		iLoop;

	for (iLoop = 0; iLoop < iPageCount; iLoop++)
	{
		CPropertyPage*	pPage = (CPropertyPage*)pPages[iLoop];

		if (pPage)
		{
			pSheet->AddPage( pPage );
		}
	}

	SaveModulePages( idModule, iPageCount, pPages );
}


void ChPageManager::SaveModulePages( const ChModuleID& idModule,
										int iPageCount, chparam* pPages )
{
	ChModulePagesInfo*	pPageInfo;

	pPageInfo = new ChModulePagesInfo( idModule, iPageCount, pPages );
	ASSERT( 0 != pPageInfo );

	m_pageInfoList.AddTail( (chparam)pPageInfo );
}


void ChPageManager::GetPageData( ChModulePagesInfo* pPagesInfo )
{
	int					iPageCount = pPagesInfo->GetPageCount();
	chparam*			pPages = pPagesInfo->GetPages();
	ChGetPageDataMsg	getDataMsg( m_type, iPageCount, pPages );

	m_pCore->DispatchMsg( pPagesInfo->GetModuleID(), getDataMsg );
}


void ChPageManager::ReleaseModulePages( ChModulePagesInfo* pPagesInfo )
{
	int					iPageCount = pPagesInfo->GetPageCount();
	chparam*			pPages = pPagesInfo->GetPages();
	ChReleasePagesMsg	relPagesMsg( m_type, iPageCount, pPages );

	m_pCore->DispatchMsg( pPagesInfo->GetModuleID(), relPagesMsg );

	delete [] pPages;
}

// $Log$
