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

	This file consists of implementation of the ChMenu class.

----------------------------------------------------------------------------*/

// $Header$

#include <string.h>

#include "headers.h"

#include <ChTypes.h>
#include <ChMenu.h>
#include "ChRMenu.h"


#include <MemDebug.h>


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define NOT_IMPLEMENTED		TRACE2( "Function not implemented, file %s: line %d.", __FILE__, __LINE__)

#if defined( CH_MSW )

	#define CH_FILE_MENU_NAME "&File"
	#define CH_FILE_MENU_CLOSE_NAME "&Close"
	#define CH_FILE_MENU_EXIT_NAME "E&xit"
	#define CH_VIEW_MENU_NAME "&View"
	#define CH_WINDOW_MENU_NAME "&Window"

#elif defined( CH_UNIX )

	#define CH_FILE_MENU_NAME "File"
	#define CH_FILE_MENU_EXIT_NAME "Exit"
	#define CH_VIEW_MENU_NAME "View"
	#define CH_WINDOW_MENU_NAME "Window"

#endif	// defined( CH_UNIX )


/*----------------------------------------------------------------------------
	class ChMenu
----------------------------------------------------------------------------*/

ChMenu::ChMenu( ChRMenuMgr* pMgr, ChString strTitle, const ChMsgHandler pDefHandler, chflag32 flOptions):
					m_pMenuMgr( pMgr ), 
					m_strTitle(strTitle), 
					m_pHandler(pDefHandler), 
					m_pBlock( 0 ), 
					m_flOptions(flOptions)
{
	 m_flState = CH_STATE_ENABLED;
	 m_idModule = 0;	// set when installed
}

ChMenu::~ChMenu()
{
	ChMenuItem*  pItem;

	while (!m_items.IsEmpty())
	{
		pItem = m_items.RemoveHead();
		delete pItem;
	}
}

ChMenuItem* ChMenu::InsertItem( ChString strTitle, const ChMsgHandler pHandler, chint16 sPosition )
{
	ChMenuItem*  pItem = new ChMenuItem( strTitle, pHandler, this );

	InsertInList( pItem, sPosition );

	return pItem;
}

ChMenuItem* ChMenu::InsertItem( ChMenu *pCascadeMenu, chint16 sPosition )
{
	ChMenuItem*  pItem = new ChMenuItem( pCascadeMenu, this );

	InsertInList( pItem, sPosition );

	return pItem;
}

ChMenuItem* ChMenu::InsertSeparator( chint16 sPosition )
{
	ChMenuItem*  pItem = new ChMenuItem( ChMenuItem::CH_MENU_ITEM_SEPARATOR, this );

	InsertInList( pItem, sPosition );

	return pItem;
}

ChMenuItem* ChMenu::FindItem( ChString strTitle, chint16 sPosition )
{
	ChPosition pos = FindItemPosition( sPosition );
	ChMenuItem*  pItem = 0;

	while( pos != NULL )
	{
    	pItem = m_items.GetNext( pos );
		if (pItem->GetTitle() == strTitle ) return pItem;
	}

	return 0;
}


ChMenuItem* ChMenu::FindItem( ChMenu *pMenu, chint16 sPosition )	// add to docs!!
{
	ChPosition pos = FindItemPosition( sPosition );
	ChMenuItem*  pItem = 0;

	while( pos != NULL )
	{
    	pItem = m_items.GetNext( pos );
		if (pItem->GetType() == ChMenuItem::CH_MENU_ITEM_CASCADE && pMenu == pItem->GetMenu() )
		{
			return pItem;
		}
	}

	return 0;
}


ChMenuItem* ChMenu::FindItem(chint16 sPosition )
{
	ChMenuItem*  pItem = 0;		   // change to call FindItemPosition

	if (CH_MENU_END == sPosition)
	{
		pItem = m_items.GetTail();
	}
	else
	{
		ChPosition pos = m_items.FindIndex( sPosition );
		if (pos)
		{
			pItem = m_items.Get( pos );
		}
	}
	return pItem;
}


ChPosition ChMenu::FindItemPosition(chint16 sPosition )
{
	ChPosition pos = 0;

	if (CH_MENU_END == sPosition)
	{
		pos = m_items.GetTailPosition();
	}
	else
	{
		 pos = m_items.FindIndex(sPosition);
	}
	return pos;
}


void ChMenu::Promote( bool boolToHead )
{
	if (IsInstalled())
	{
		m_pBlock->Promote( this, boolToHead );
	}
	else
	{
		NOT_IMPLEMENTED;					// throw??	 (programmer error)
	}
}


void ChMenu::Enable( bool boolEnable )
{
	bool isEnabled;

	isEnabled = (m_flState &  CH_STATE_ENABLED) != 0;
	if (boolEnable)
	{
		m_flState |= (chflag32)CH_STATE_ENABLED;
	}
	else
	{
		m_flState &= ~(chflag32)CH_STATE_ENABLED;
	}

	if ((isEnabled != boolEnable) && IsInstalled())
	{
		NOT_IMPLEMENTED;
	}
}


/*----------------------------------------------------------------------------
	ChMenu::Install
------------------------------------------------------------------------------

	Installs a menu	for module idModule.

----------------------------------------------------------------------------*/

ChMenu* ChMenu::Install( const ChModuleID idModule,	bool boolToHead )
{
	return GetMenuMgr()->Install( this, idModule, boolToHead );
}


ChMenu* ChMenu::Install( const ChString& strModuleName, bool boolToHead)
{
	return GetMenuMgr()->Install( this, strModuleName, boolToHead );
}

void ChMenu::Uninstall()
{
	if (m_pBlock)
	{
		ChRMenu *pRMenu = m_pBlock->GetRMenu();
		if(m_pBlock->GetRMenu()->Uninstall( this, m_pBlock ))
		{
			delete pRMenu;
		}
	}
	m_pBlock = 0;
}

void ChMenu::SetTitle( ChString strTitle )
{
	// needs to deinstall and reinstall

	if (IsInstalled())
	{
		NOT_IMPLEMENTED;
	}
	else
	{
		m_strTitle = strTitle;
	}
}



/*----------------------------------------------------------------------------
	ChMenu derived classes: predefined menus
	These need work to be internationalized!!
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
	ChHelpMenu publics
----------------------------------------------------------------------------*/

ChHelpMenu::ChHelpMenu( ChRMenuMgr* pMgr, ChMsgHandler pHandler, chflag32 flOptions ) :
				ChMenu( pMgr, "&Help", pHandler, flOptions )
{
	ChString		strName;
	
	strName = GetMenuMgr()->GetItem( ID_HELP_INDEX )->GetTitle();
	this->InsertItem( strName );
}


/*----------------------------------------------------------------------------
	ChEditMenu publics
----------------------------------------------------------------------------*/

ChEditMenu::ChEditMenu( ChRMenuMgr* pMgr, ChMsgHandler pHandler, chflag32 flOptions ) :
				ChMenu( pMgr, "&Edit", pHandler, flOptions )
{
	ChString		strName;
	
	strName = GetMenuMgr()->GetItem( ID_EDIT_CUT )->GetTitle();
	this->InsertItem( strName );

	strName = GetMenuMgr()->GetItem( ID_EDIT_COPY )->GetTitle();
	this->InsertItem(strName);

	strName = GetMenuMgr()->GetItem( ID_EDIT_PASTE )->GetTitle();
	this->InsertItem( strName );
}


/*----------------------------------------------------------------------------
	ChFileMenu publics
----------------------------------------------------------------------------*/

ChFileMenu::ChFileMenu( ChRMenuMgr* pMgr, ChMsgHandler pHandler, chflag32 flOptions ) :
				ChMenu( pMgr, CH_FILE_MENU_NAME, pHandler, flOptions )
{
	ChString		strName;
	ChRMenuItem	*pRMenuItem;

	pRMenuItem = GetMenuMgr()->GetItem( ID_FILE_CLOSE );
	if (pRMenuItem)
		strName = pRMenuItem->GetTitle();
	else
		strName = CH_FILE_MENU_CLOSE_NAME;
	this->InsertItem( strName );
}


/*----------------------------------------------------------------------------
	ChViewMenu publics
----------------------------------------------------------------------------*/

ChViewMenu::ChViewMenu( ChRMenuMgr* pMgr, ChMsgHandler pHandler, chflag32 flOptions ) :
				ChMenu( pMgr, CH_VIEW_MENU_NAME, pHandler, flOptions )
{
}


/*----------------------------------------------------------------------------
	ChWindowMenu publics
----------------------------------------------------------------------------*/

ChWindowMenu::ChWindowMenu( ChRMenuMgr* pMgr, ChMsgHandler pHandler, chflag32 flOptions ) :
				ChMenu( pMgr, CH_WINDOW_MENU_NAME, pHandler, flOptions )
{
}


/*----------------------------------------------------------------------------
	ChMenu private or protected helper methods
----------------------------------------------------------------------------*/

ChMenuItem* ChMenu::FindItemByID(chuint32 id)
{
	ChString strName = GetMenuMgr()->GetItem(id)->GetTitle();
	return( FindItem(strName) );
}

ChMenuItem* ChMenu::InsertInList( ChMenuItem* pItem, chint16 sPosition)
{
	if (sPosition >= m_items.GetCount()) sPosition = CH_MENU_END;

	if (CH_MENU_END == sPosition)
	{
		m_items.AddTail( pItem );
	}
	else
	{
		ChPosition pos = m_items.FindIndex( sPosition );
		m_items.InsertBefore( pos, pItem );
	}

	if (IsInstalled())
	{
		//chuint32 luAt = -1;
		m_pBlock->InstallItem( pItem, 0, CH_MENU_AT_EXISTING );
	}

	return pItem;
}

void ChMenu::RemoveItem(ChMenuItem* pItem)
{
	ChPosition	pos = m_items.Find( pItem );

	if (pos)
	{
		m_items.Remove( pos );
	}
}

/*----------------------------------------------------------------------------
	ChMenuItem constructors and destructor
----------------------------------------------------------------------------*/

ChMenuItem::ChMenuItem( ChString strTitle, const ChMsgHandler pHandler,
						ChMenu *pParent ) :
 				m_strTitle( strTitle ),
 				m_strHelp( "" ), m_pParent( pParent ), m_type( CH_MENU_ITEM_TEXT ),
				m_pRItem( 0 ), m_fsAccMods(0),	m_luAccChar(0)
{
	//m_flState = CH_STATE_ENABLED;		  	// this is normal, enabled, but not checked
	m_flState = 0;		  	// this is normal, enabled, but not checked
	m_pHandler = pHandler;
	if (pHandler)
	{
		m_ppHandler = &m_pHandler;
	}
	else
	{
		m_ppHandler = pParent->GetHandlerAddr();
	}
}

ChMenuItem::ChMenuItem( ChMenu *pCascadeMenu,  ChMenu *pParent ):
		m_strHelp(""),
		m_pCascade(pCascadeMenu),
		m_pParent(pParent),
		m_type(CH_MENU_ITEM_CASCADE),
		m_pRItem( 0 ),
		m_fsAccMods(0),	m_luAccChar(0)
{
	m_pHandler = 0;
	m_ppHandler = pParent->GetHandlerAddr();
	//m_flState = CH_STATE_ENABLED;		  	// this is normal, enabled, but not checked
	m_flState = 0;		  	// this is normal, enabled, but not checked
	m_strTitle = pCascadeMenu->GetTitle();
}

ChMenuItem::ChMenuItem( ChMenuItem::ItemType eType, ChMenu *pParent ):
	m_strHelp(""), m_pParent(pParent), m_pRItem( 0 ),
	m_fsAccMods(0),	m_luAccChar(0)
{
	m_ppHandler = pParent->GetHandlerAddr();
	//m_flState = CH_STATE_ENABLED;		  	// this is normal, enabled, but not checked
	m_flState = 0;		  	// this is normal, enabled, but not checked
	if (CH_MENU_ITEM_SEPARATOR == eType)
	{
		m_type = CH_MENU_ITEM_SEPARATOR;
	}
	else
	{
		NOT_IMPLEMENTED;	   // Throw??
	}
}


ChMenuItem::~ChMenuItem()
{
	if (m_pRItem) m_pRItem->GetBlock()->DeleteItem(m_pRItem, this);
	if (m_pParent)
	{
		m_pParent->RemoveItem(this);
		m_pParent = 0;
	}
}

ChMenuItem *ChMenuItem::Enable( bool boolEnable, bool boolSilent )
{
	bool isEnabled;

	isEnabled = (m_flState &  CH_STATE_ENABLED) != 0;
	if (boolEnable)
	{
		m_flState |= (chflag32)CH_STATE_ENABLED;
	}
	else
	{
		m_flState &= ~(chflag32)CH_STATE_ENABLED;
	}

	// VSP : (isEnabled != boolEnable) commented code after consulting with Jim. 5/9/95
	// For some reason isEnabled is true when it should be false, this will cause it to sync
	// with the current state.
	if (!boolSilent && /*(isEnabled != boolEnable) && */ m_pParent->IsInstalled())
	{
		m_pRItem->Enable( boolEnable );
	}
	return this;
}

ChMenuItem * ChMenuItem::Check( bool boolCheck, bool boolSilent )
{
	bool isChecked;

	isChecked = (m_flState &  CH_STATE_CHECKED) != 0;
	if (boolCheck)
	{
		m_flState |= (chflag32)CH_STATE_CHECKED;
	}
	else
	{
		m_flState &= ~(chflag32)CH_STATE_CHECKED;
	}

	if (!boolSilent && (isChecked != boolCheck) && m_pParent->IsInstalled())
	{
		m_pRItem->Check(boolCheck);
	}

	return this;
}

ChMenuItem * ChMenuItem::SetTitle(ChString strTitle)
{
	m_strTitle = strTitle;

	if ( m_pParent->IsInstalled())
	{
		// It seems like this should cause a de-install
		// and re-install of theitem
		NOT_IMPLEMENTED;
	}
	return this;
}

ChMenuItem * ChMenuItem::SetHandler(const ChMsgHandler pHandler)
{
	// Note that this -does not- affect RItem if installed
	m_pHandler = pHandler;
	if (pHandler)
	{
		m_ppHandler = &m_pHandler;
	}
	else
	{
		m_ppHandler = m_pParent->GetHandlerAddr();
	}
	return this;
}

ChMenuItem * ChMenuItem::SetHelpText(ChString strHelp)
{
	// Note that this -does not- affect RItem if installed
	m_strHelp = strHelp;
	return this;
}

ChMenuItem* ChMenuItem::SetAccelerator( const chuint32 luChar, const chflag16 fsMods )
{
	m_fsAccMods = fsMods;
	m_luAccChar = luChar; 
	return this;
}


chint16 ChMenuItem::GetIndex()
{
	chint16	sIndex;

	sIndex = m_pParent->GetItemIndex( this );

	return sIndex;
}

// Helper function for ChMenuItem::GetIndex
chint16 ChMenu::GetItemIndex(ChMenuItem*  pItem)
{
	chint16	sIndex;
	ChMenuItem*  pItemInList = 0;
	ChPosition pos;

	for( pos = m_items.GetHeadPosition(), sIndex = 0; pos != NULL; sIndex++)
	{
    	pItemInList = m_items.GetNext( pos );
		if (pItem == pItemInList ) break;
	}
	if (!pos)
	{
		// exception??
		NOT_IMPLEMENTED;
	}

	return sIndex;
}

// $Log$
