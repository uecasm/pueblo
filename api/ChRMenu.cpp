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

	This file consists of implementation of the ChRMenu, ChRMenuItem,
	ChRMenuBlock, and ChRMenuMgr  classes. These are interanl helper classes
	ChMenu and its kin.

----------------------------------------------------------------------------*/

// $Header$

#include <string.h>

#include "headers.h"

#include <ChTypes.h>
#include <ChMenu.h>

#include "ChRMenu.h"

#include <ChCore.h>

#include <ChMsgTyp.h>
#ifdef CH_UNIX
#include <strstream>
#include <Xm/RowColumn.h>
#include <Xm/CascadeB.h>
#include <Xm/SeparatoG.h>
#include <Xm/PushBG.h>
#else
#include <strstream>
#endif            

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define NOT_IMPLEMENTED		TRACE2( "Function not implemented, file %s: line %d.", __FILE__, __LINE__)
#define MAX_MENU_NAME	255


#if !defined( NO_TEMPLATES )
template <class TYPE>
void ThrowIfEmptyMenu( TYPE* a )
{
	#if defined( CH_EXCEPTIONS )
	{
		if (a->IsEmpty())
		{
			throw ChMenuEx();  // need our own exceptions
		}
	}
	#endif	// defined( CH_EXCEPTIONS )
}  
#else
void ThrowIfEmptyMenu( void* a )
{
} 
#endif


/*----------------------------------------------------------------------------
	ChMenu constructor and destructor
----------------------------------------------------------------------------*/

ChRMenu::ChRMenu( ChRMenuMgr* pMenuMgr, const ChString& strTitle, ChNativeMenu * pNatMenu,
					const ChMsgHandler pHandler  ) :
					m_pMenuMgr( pMenuMgr ),
					m_strTitle(strTitle)
{
	// This method takes existing native menu and gets it into our data structures
	// It assumes the menu is to be shared

	m_pParent = 0;	// for now, until we do cascades

	{
		ChMenu*			pMenu;
		ChRMenuBlock*	pBlock;

#if defined( CH_MSW )
											// pMenu is really a CMenu...
		m_hMenu =  pNatMenu->GetSafeHmenu();
#endif // CH_MSW
											/* Create a ChMenu and
												ChRMenuBlock, and populate
												them */
		pMenu = new ChMenu( pMenuMgr, strTitle, pHandler );
		m_pMyMenu = pMenu;
        ChString	strTemp = "";
		pBlock = new ChRMenuBlock( strTemp, this );	// shared
		m_MenuBlocks.AddTail( pBlock );
											/* Now install it, taking special
												care with the items, which
												already have ids */
		pBlock->Init( pNatMenu, pMenu, true );
	}
}

#ifdef CH_UNIX
void ChRMenuItem_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
	ChRMenuItem *pRItem = (ChRMenuItem *)client_data;

	ChRMenuMgr::GetMgr()->Notify( pRItem->GetId() );
}
#endif

ChRMenu::ChRMenu( ChRMenuMgr* pMenuMgr, const ChString& strTitle, ChNativeMenu * pMenuBar  ):
					m_pMenuMgr( pMenuMgr ),
					m_pMyMenu(0),
					m_strTitle(strTitle)
{
#ifdef CH_MSW
	chuint	uPos;
#endif

	m_pParent = 0;	// for now, until we do cascades
	m_boolKeepHMenu = false;

#ifdef CH_MSW
	m_hMenu = CreatePopupMenu();

	// Add it to the menubar right before "Help"; we don't have a "Window"
	uPos = pMenuBar->GetMenuItemCount() - 1;
	uPos = max(uPos, 0);
	pMenuBar->InsertMenu( uPos, MF_BYPOSITION | MF_POPUP, (UINT)m_hMenu, strTitle);
	GetMenuMgr()->GetCore()->GetFrameWnd()->DrawMenuBar();
#else
	// Make the new menu.  The parent widget is *pMenuBar.
	Widget pulldown;
	pulldown = XmCreatePulldownMenu( *pMenuBar, "foopulldown", NULL, 0 );
	m_hMenu = XtVaCreateManagedWidget( strTitle,
					   xmCascadeButtonWidgetClass, *pMenuBar,
					   XmNsubMenuId, pulldown,
					   NULL );
	XtManageChild(m_hMenu);
#endif

}

// Constructor for cascade menu
ChRMenu::ChRMenu( ChRMenuMgr* pMenuMgr, const ChString& strTitle, ChRMenuItem *pItem  ):
					m_pMenuMgr( pMenuMgr ),
					m_pMyMenu(0),
					m_strTitle(strTitle)
{
	m_pParent = 0;	//  ?? do we need??
	m_pRItem = pItem;
	m_boolKeepHMenu = false;
#ifdef CH_MSW
	m_hMenu = CreatePopupMenu();
#else
	NOT_IMPLEMENTED;
#endif
}

ChRMenu::~ChRMenu()
{
	ChRMenuBlock*	pBlock;
											/* Delete the hmenu if not
												no-deleted */
	{
		if (!m_boolKeepHMenu)
		{
			GetMenuMgr()->RemoveRMenu(m_strTitle); 
#if defined CH_MSW
			ChNativeMenu*	pBar;
			chint32			lCount;
			chint32			lLoop;

			pBar = ChNativeMenu::FromHandle( GetMenuMgr()->GetBarHandle());

											/* Note that MFC will dispose of
												a temporary CMenu */
			lCount = pBar->GetMenuItemCount();

			for (lLoop = 0; lLoop < lCount; lLoop++)
			{
				if (pBar->GetSubMenu( (UINT)lLoop )->GetSafeHmenu() == m_hMenu)
				{
					pBar->DeleteMenu( (UINT)lLoop, MF_BYPOSITION );
					break;
				}
			}
			GetMenuMgr()->GetCore()->GetFrameWnd()->DrawMenuBar();
#else
			XtDestroyWidget( m_hMenu );
#endif	// CH_MSW
		}
	}

	while (!m_MenuBlocks.IsEmpty())
	{
    	pBlock = m_MenuBlocks.GetHead();
		delete pBlock;
		m_MenuBlocks.RemoveHead();
	}

	delete m_pMyMenu;
}

/*----------------------------------------------------------------------------
	ChRMenu public methods
----------------------------------------------------------------------------*/
ChMenu* ChRMenu::Install( ChMenu *pMenu,  const ChString& strModuleName, bool boolToHead)
{
	ChString	strBlockModuleName;
	chuint32	luCount = 0;

	if (pMenu->IsInstalled())
	{
		// or better yet, throw??
		return 0;
	}

	if (pMenu->IsDelimited())
	{
		strBlockModuleName = strModuleName;
	}
	else
	{
		strBlockModuleName = "";
	}
	ChRMenuBlock *pBlock = FindBlock( strBlockModuleName, &luCount );

	if(!pBlock)
	{
		// make a new block
		pBlock = new ChRMenuBlock( strBlockModuleName, this );
		m_MenuBlocks.AddTail( pBlock );
	}

	#if defined CH_MSW
		ChNativeMenu * pNatMenu = CMenu::FromHandle(m_hMenu);
	#else
		ChNativeMenu * pNatMenu = &m_hMenu;
	#endif // CH_MSW
	pBlock->Install( pNatMenu, pMenu, luCount, boolToHead );
	return (pMenu);
}


void ChRMenu::Promote( ChMenu *pMenu, bool boolToHead )
{
	(pMenu->GetBlock())->Promote( pMenu, boolToHead );
}


bool ChRMenu::Uninstall( ChMenu *pMenu, ChRMenuBlock *pBlock )
{
	bool boolDeleteMe = false;

	// Remove pMenu items from pBlock
	pBlock->Uninstall( pMenu );

	// Remove block if empty
	if (pBlock->GetCount() == 0)
	{
		ChPosition	pos;

		delete pBlock;

		if (pos = m_MenuBlocks.Find( pBlock ))
		{
			m_MenuBlocks.Remove( pos );		// delete from list, too
		}
	}
											 /* Wipe the native menu free of any
											 leftover separators */
	CleanupSeparators();
											/* Return true if we should be
												deleted */
	boolDeleteMe = m_MenuBlocks.GetCount() == 0;

	return boolDeleteMe;
}


/*----------------------------------------------------------------------------
	ChRMenu private or protected helper methods
----------------------------------------------------------------------------*/

ChRMenu* ChRMenu::CleanupSeparators()
{
	ChNativeMenu *pNatMenu = GetNative();

	int count = pNatMenu->GetMenuItemCount( );
	bool	boolSeparator = true;
	for(int i = count - 1; i >= 0; i--)
	{
		if(pNatMenu->GetMenuState(i, MF_BYPOSITION) & MF_SEPARATOR)
		{
			if(boolSeparator || i == 0)
			{
				pNatMenu->DeleteMenu((UINT)i, MF_BYPOSITION);
			}
			else
			{
				boolSeparator = true;
			}	
		}	
		else
		{
			boolSeparator = false;
		}	
	}
	return this;
}

ChRMenuBlock* ChRMenu::FindBlock(ChString strModName, chuint32 *plBegin)
{
	ChRMenuBlock*	pBlock;
	ChPosition		pos = m_MenuBlocks.GetHeadPosition();
	chuint32		luCount = 0;

											/* Linear search - candidate for
												optimizing */
	while (pos != NULL)
	{
    	pBlock = m_MenuBlocks.GetNext( pos );
		if (pBlock->GetModuleName() == strModName )
		{
			if (plBegin) *plBegin = luCount;
			return pBlock;
		}
		if (plBegin) luCount += pBlock->GetCount();
	}

	if (plBegin) *plBegin = luCount;
	return (0);
}



/*----------------------------------------------------------------------------
	ChRMenuItem constructors and destructor
----------------------------------------------------------------------------*/

ChRMenuItem::ChRMenuItem( ChRMenuBlock *pBlock, ChMenuItemId id ) :
				m_id( id ), m_pBlock( pBlock ), m_fsAccMods(0),
				m_luAccChar(0)
#ifdef CH_UNIX
	 			, m_widget(0)
#endif
{
	ASSERT( m_pBlock );
}

ChRMenuItem::~ChRMenuItem()
{
	ChPosition		pos = m_items.GetHeadPosition();
	ChMenuItem*		pItem = 0;
	
	while (0 != pos)
	{
		pItem = m_items.GetNext( pos );
		pItem->SetRItem( 0 );
	}
}

chint32 ChRMenuItem::GetCount()
{
	return m_items.GetCount();
}

ChRMenuMgr* ChRMenuItem::GetMenuMgr() const
{
	return GetBlock()->GetRMenu()->GetMenuMgr();
}



// these set/get common data; since we don't store it ourselves, go to foremost for get's
void ChRMenuItem::Enable(bool boolEnable)
{
	#if defined CH_MSW
		ChNativeMenu *pBar = ChNativeMenu::FromHandle(GetMenuMgr()->GetBarHandle());

		pBar->EnableMenuItem((UINT)m_id, MF_BYCOMMAND | (boolEnable ? MF_ENABLED : MF_GRAYED ));
		// mfc will dispose of temporary cmenu *
	#else
		XtSetSensitive( GetWidget(), boolEnable );
	#endif

	ChPosition pos = m_items.GetHeadPosition(  );
	ChMenuItem*  pItem = 0;

	while( pos != NULL )
	{
    	pItem = m_items.GetNext( pos );
		pItem->Enable(boolEnable, true);
	}
	NotifyChanged();
	return;
}

bool ChRMenuItem::IsEnabled()
{   
	ThrowIfEmptyMenu(&m_items);
	return ((m_items.GetHead())->IsEnabled());
}

void ChRMenuItem::Check(bool boolCheck)
{
	#if defined CH_MSW
		ChNativeMenu *pBar = ChNativeMenu::FromHandle(GetMenuMgr()->GetBarHandle());

		pBar->CheckMenuItem((UINT)m_id, MF_BYCOMMAND | (boolCheck ? MF_CHECKED : MF_UNCHECKED ));
		// mfc will dispose of temporary cmenu *
	#else
		NOT_IMPLEMENTED;
	#endif

	ChPosition pos = m_items.GetHeadPosition();
	ChMenuItem*  pItem = 0;

	while( pos != NULL )
	{
    	pItem = m_items.GetNext( pos );
		pItem->Check(boolCheck, true);
	}
	NotifyChanged();
	return;
}

bool ChRMenuItem::IsChecked()
{
	ThrowIfEmptyMenu(&m_items);
	return ((m_items.GetHead())->IsChecked());
}

ChString& ChRMenuItem::GetTitle()
{
	ThrowIfEmptyMenu(&m_items);
	return ((m_items.GetHead())->GetTitle());
}

ChMenuItem::ItemType ChRMenuItem::GetType()
{
 	ThrowIfEmptyMenu(&m_items);
	return ((m_items.GetHead())->GetType());
}

// these return data for foremost
ChString& ChRMenuItem::GetHelpText()
{
 	ThrowIfEmptyMenu(&m_items);
	return ((m_items.GetHead())->GetHelpText());
}

ChMenu *ChRMenuItem::GetMenu()
{
 	ThrowIfEmptyMenu(&m_items);
	return ((m_items.GetHead())->GetMenu());
}

// Item/item list manipulators
void ChRMenuItem::Add(ChMenuItem *pMenuItem, bool boolToHead )
{
	if(boolToHead)
	{
		m_items.AddHead(pMenuItem);
	}
	else
	{
		m_items.AddTail(pMenuItem);
	}

	pMenuItem->SetRItem(this);
	m_strDisplayTitle = MakeDisplayTitle();

	return;
}

void ChRMenuItem::Add(ChMenuItem *pMenuItem, chuint16 suWhere, ChMenuSplay *pSplayAfter )
{
	if(suWhere == CH_MENU_AT_HEAD || m_items.GetCount() == 0)
	{
		m_items.AddHead(pMenuItem);
	}
	else if(suWhere == CH_MENU_AT_TAIL)
	{
		m_items.AddTail(pMenuItem);
	}
	else
	{
		// insert -before- any of pMenu's in tree
		ChPosition pos = m_items.GetHeadPosition();
		ChMenuItem*  pItem = 0;
		bool boolDone = false;
		while( pos != NULL )
		{
			ChPosition oldPos = pos;
	    	pItem = m_items.GetNext( pos );
			ChMenu *pMenu = pItem->GetMenu();
			if(pSplayAfter->Find(pMenu))
			{
				m_items.InsertBefore(oldPos, pMenuItem);
				boolDone = true;
				break;	// found
			}
		}
		if(!boolDone)
		{
			m_items.AddTail(pMenuItem);
		}
	}

	pMenuItem->SetRItem(this);
	m_strDisplayTitle = MakeDisplayTitle();

	return;
}

ChString  ChRMenuItem::MakeDisplayTitle( )
{
	ChString strDispTitle = GetTitle();
	ChPosition pos = m_items.GetHeadPosition();
	
	while( pos != NULL )
	{
    	ChMenuItem *pItem = m_items.GetNext( pos );
		if(pItem->GetAccelerator())
		{
			strDispTitle = pItem->GetDisplayTitle();
			break;	// found
		}
	}
	return strDispTitle;
}

// Update the accelerator for this ritem with the one assigned to this item
// remember, first one in takes precedence.
void ChRMenuItem::AddAccelerator(const ChMenuItem * pItem)
{
	// Mow add to the accelerator table if necessary
	if (!m_luAccChar)		// first has precedence
	{
		m_luAccChar = pItem->GetAccelerator( m_fsAccMods );
		if (m_luAccChar)
		{
			m_pBlock->AddAccelerator( m_id, m_luAccChar, m_fsAccMods );
		}
	}
}									

ChString ChMenuItem::GetDisplayTitle()
{
	ChString	strAccel = m_strTitle;

	if (m_luAccChar)
	{ 
		strAccel += "\t";

		if (m_fsAccMods & CH_ACC_SHIFT)
		{
			strAccel += "Shift+";
		}

		if (m_fsAccMods & CH_ACC_CONTROL)
		{
			strAccel += "Ctrl+";
		}

		if (m_fsAccMods & CH_ACC_ALT)
		{
			strAccel += "Alt+";
		}

#ifdef CH_MSW
		if (m_fsAccMods & CH_ACC_VIRTKEY)
		{
		    if ((m_luAccChar >= 'A' || m_luAccChar <= 'Z') ||
				(m_luAccChar >= '0' || m_luAccChar <= '9'))
			{
				strAccel += (TCHAR)m_luAccChar;
			}
			else
			{
				TCHAR	strBuf[100];

				GetKeyNameText( (long)m_luAccChar, strBuf, 100 - 1 );
				strAccel += strBuf;
			}
		}
		else
		{
			strAccel += (TCHAR)m_luAccChar;
		}
#else
		NOT_IMPLEMENTED;
#endif
		
	}

	return strAccel;
}

ChString  ChRMenuItem::SplitDisplayTitle(const ChString & strDisplayTitle )
{
#ifdef CH_MSW
	ChString strTitle = strDisplayTitle.SpanExcluding("\t");
	return strTitle;
#else
	NOT_IMPLEMENTED;
	return "";
#endif
}

void ChRMenuItem::Promote( ChMenu *pMenu, bool boolToHead )
{
											// needed later??
	NOT_IMPLEMENTED;
}

void ChRMenuItem::Promote( ChMenuItem *pMenuItem, bool boolToHead )
{
											// need this first
	ChPosition	pos;

	if (pos = m_items.Find( pMenuItem ))
	{
		m_items.Remove( pos );
	}

	if (boolToHead)
	{
		m_items.AddHead( pMenuItem );
	}
	else
	{
		m_items.AddTail( pMenuItem );
	}
}

void ChRMenuItem::Delete( ChMenu *pMenu )
{
	ChPosition	pos = m_items.GetHeadPosition(  );
	ChPosition	oldPos = pos;
	ChMenuItem*	pItem = 0;
	
	while( pos != NULL ) {
		pItem = m_items.GetNext( pos );
		if (pItem->GetParent() == pMenu) {
			m_items.Remove( oldPos );
			pItem->SetRItem( 0 );
			break;
		}
		oldPos = pos;
	}
}

void ChRMenuItem::Delete( ChMenuItem *pMenuItem )
{
	#if defined( CH_MSW )
	{
		ChPosition	pos;

		if (pos = m_items.Find( pMenuItem ))
		{
			m_items.Remove( pos );
		}
		pMenuItem->SetRItem( 0 );
		// Delete accelerator. jwd 1/11/96
		GetBlock()->DeleteAccelerator( m_id );
	}
	#else	// defined( CH_MSW )
	{
		NOT_IMPLEMENTED;
	}
	#endif	// defined( CH_MSW )
}


// Notification -- Return whether or not item should be enabled

void ChRMenuItem::Notify( ChMenuMsg& msg, bool boolStopOnProcessed,
							bool* pboolEnabled )
{
	ChPosition	pos = m_items.GetHeadPosition();
	ChMenuItem*	pItem = 0;

	while (0 != pos)
	{
    	pItem = m_items.GetNext( pos );

		msg.SetItem( pItem );
		msg.SetParam1( true );
		msg.SetMenu( pItem->GetParent() );

		(pItem->GetHandler())( msg, 0 );

		if (boolStopOnProcessed && msg.IsProcessed())
		{
			if (pboolEnabled)
			{
				*pboolEnabled = pItem->IsEnabled();
			}
			break;
		}
	}
}

void ChRMenuItem::NotifyChanged()
{
	ChMenuMsg msg( CH_MSG_MENU_CHANGED );

	Notify( msg, false );
}


/*----------------------------------------------------------------------------
	ChRMenuMgr constructors and destructor and other methods
----------------------------------------------------------------------------*/

ChRMenuMgr::ChRMenuMgr( ChCore* pCore ) :
				m_pCore( pCore )
{
	ASSERT( m_pCore );
	m_nextId = CH_MENU_FIRST_ID;
//	m_pTheMgr = this;	// only one in the world
}


ChRMenuMgr::~ChRMenuMgr()
{											/* Walk the tree, deleting all
												the RMenus */
	ChDeleteRMenuOp		zapRMenu;

	m_menus.Infix( zapRMenu );
//	m_pTheMgr = 0;
}


#if 0
// return the only allowed instatnce
ChRMenuMgr *ChRMenuMgr::GetMgr()
{
	return m_pTheMgr;
}
#endif

// Finds the right ChRMenu, then puts it in; adds elements
// to ChRMenuItem lists
ChMenu* ChRMenuMgr::Install( ChMenu *pMenu, const ChString& strModuleName, bool boolToHead )
{
	ChRMenu	*pRMenu, **ppRMenu;

	// Find the ChRMenu by name
	if (! (ppRMenu = (m_menus.Find(pMenu->GetTitle()))))
	{
		// not found, it's a new RMenu

#ifdef CH_MSW
		pRMenu = new ChRMenu( this, pMenu->GetTitle(), CMenu::FromHandle(m_hBar) );
#else
		pRMenu = new ChRMenu( this, pMenu->GetTitle(), &m_hBar );
#endif
		m_menus.Insert(pMenu->GetTitle(), pRMenu);
	}
	else
	{
		pRMenu = *ppRMenu;
	}

	// Install it in the RMenu
	pRMenu->Install(pMenu, strModuleName, boolToHead);
	return (pMenu);
}

ChMenu* ChRMenuMgr::Install( ChMenu *pMenu, const ChModuleID idModule, bool boolToHead )
{
	ChString strModuleName = GetCore()->GetModuleName(idModule);

	return (Install( pMenu, strModuleName, boolToHead ));
}

// Initialize, absorbing the existing menuBar, and assocating it with some module
void ChRMenuMgr::Init( ChNativeMenu *pBar, const ChModuleID idModule,  
	const ChMsgHandler pHandler, const HACCEL m_hAccelTable )
{
#if defined(CH_MSW)
	// pBar is really a CMenu...
	m_hBar =  pBar->GetSafeHmenu();
	chint16 sCount = pBar->GetMenuItemCount();
	// Walk the bar; add each menu
	LPTSTR pBuf = new TCHAR[MAX_MENU_NAME + 1];
	for (int i = 0; i < sCount; i++ )
	{
		ChString	strTitle;

		CMenu *pMenu =  pBar->GetSubMenu(i);
		pBar->GetMenuString(i, pBuf, MAX_MENU_NAME, MF_BYPOSITION);
		ChRMenu *pRMenu = new ChRMenu( this, pBuf, pMenu, pHandler );
		m_menus.Insert(pBuf, pRMenu);
	}
	delete []pBuf;
#elif defined(CH_UNIX)
	m_hBar = *pBar;
#endif // CH_UNIX
}

ChMenuItemId ChRMenuMgr::MakeId()
{
	while( true )
	{
		m_nextId++;
		if (CH_MENU_LAST_ID < m_nextId)
		{
			m_nextId = CH_MENU_FIRST_ID;
		}

		if (!m_itemTree.Find( m_nextId ))
			break;
	}
	return m_nextId;
}

ChRMenuItem *ChRMenuMgr::GetItem( ChMenuItemId itemId )
{
	ChRMenuItem*	pRItem;

	if (pRItem = m_itemTree.FindValue( itemId ))
	{
		return pRItem;
	}
	else
	{
		return 0;
	}
}

void ChRMenuMgr::DeleteItem( ChMenuItemId id )
{											// Remove from the tree
	m_itemTree.Delete( id );
}

ChRMenuItem *ChRMenuMgr::PutItem( ChMenuItemId id, ChRMenuItem *pRItem )
{
	if (m_itemTree.Find( id ))
	{
		#if defined( CH_EXCEPTIONS )
		{
			#if defined( CH_MSW) && defined( CH_ARCH_16 )  
			{  
				THROW( new ChMenuEx());
			}
			#else
			throw ChMenuEx();		// must be unique 
			#endif
		}
		#else
		{
			return 0;
		}
		#endif
	}

	m_itemTree.Insert( id, pRItem );

	return pRItem;
}

ChRMenu *ChRMenuMgr::GetRMenu(ChString strTitle)
{
	return (*(m_menus.Find(strTitle)));
}

bool ChRMenuMgr::Notify( ChMenuItemId id, chuint32 uMessage,
							bool* pboolEnabled )
{
	ChRMenuItem*	pRItem = m_itemTree.FindValue( id );
	bool			boolEnabled = false;

	if (0 == pboolEnabled)
	{
		pboolEnabled = &boolEnabled;
	}
	else
	{
		*pboolEnabled = false;
	}

	if (pRItem)
	{
		ChMenuMsg msg( uMessage );

		pRItem->Notify( msg, true, pboolEnabled );
	}
	else if (pboolEnabled)
	{
		*pboolEnabled = true;
	}

	return *pboolEnabled;
}

bool ChRMenuMgr::GetMessageString( UINT nID, ChString& rMessage )
{
	ChRMenuItem *pRItem = GetItem( nID );

	if (0 == pRItem) return false;

	rMessage = pRItem->GetHelpText();

	if ( (CH_MENU_FIRST_ID > nID || CH_MENU_LAST_ID < nID) &&  rMessage == "") return false;
	return true;
}

#ifdef CH_MSW
void ChRMenuMgr::RegisterAcceleratorTable( HACCEL hAccel )
{
	chparam val = (chparam)hAccel;
	
	// search the list for a match; add if not found
	if (m_accelerators.IsEmpty() || 0 == m_accelerators.Find(val))
	{
		m_accelerators.AddTail(val);
	}
	return;
}

void ChRMenuMgr::UnregisterAcceleratorTable( HACCEL hAccel )
{
										// search the list for a match; delete if not found
	chparam val = (chparam)hAccel;
	
	ChPosition pos = m_accelerators.Find(val);
	if (pos)
	{
		m_accelerators.Remove(pos);
	}
	return;
}

int ChRMenuMgr::TranslateAccelerators( HWND hWnd, LPMSG lpmsg )
{
	int	iRetVal = false;
	
	if (!m_accelerators.IsEmpty())
	{
		HACCEL hAccel;
 		ChPosition pos = m_accelerators.GetHeadPosition();
		while (pos)
		{
			hAccel = (HACCEL)m_accelerators.GetNext(pos);
			iRetVal = ::TranslateAccelerator(hWnd, hAccel, lpmsg );
			if (iRetVal) break;
		}

	}
	return iRetVal;
}
#endif // CH_MSW

/*----------------------------------------------------------------------------
	ChRMenuBlock public methods
----------------------------------------------------------------------------*/

ChRMenuBlock::ChRMenuBlock( ChString strModName, ChRMenu *pRMenu  ) :
					m_strModuleName( strModName ), m_pContainedIn( pRMenu ),
					m_hAccelTable(0)
{		 
	ASSERT( pRMenu );
}

ChRMenuBlock::~ChRMenuBlock()
{											// Get rid of all the items
	ChDeleteBlocksOp	zapBlocks;

	m_itemNames.Infix( zapBlocks );
	if (m_hAccelTable)
	{
#ifdef CH_MSW
		m_pContainedIn->GetMenuMgr()->UnregisterAcceleratorTable(m_hAccelTable);
		::DestroyAcceleratorTable( m_hAccelTable );
#else
		// No Accelerator table on Unix
#endif
		m_hAccelTable = 0;
	}

}


ChMenu* ChRMenuBlock::Install( ChNativeMenu* pNatMenu, ChMenu* pMenu,
								chuint32 luBegin, bool boolToHead )
{
	chuint32	luAt = luBegin;
	if (GetCount() == 0 && luBegin > 0)
	{
#ifdef CH_MSW
		// Add extra separator
		// First add the item to the hMenu
		pNatMenu->InsertMenu((UINT)luAt++, MF_BYPOSITION | MF_SEPARATOR);

		// Now add to the block, and the mgrs list
		ChMenuItemId id = GetRMenu()->GetMenuMgr()->MakeId();
		ChRMenuItem *pRItem = new ChRMenuItem( this, id );

		m_itemNames.Insert(MakeSeparatorTitle(id), pRItem);
		GetRMenu()->GetMenuMgr()->PutItem(id, pRItem);
#else
		NOT_IMPLEMENTED;
#endif
	}

	// Finally, honest-to-goodness, we add the menu to a block, and add items
	if(boolToHead)
	{
		m_installedMenus.AddHead(pMenu);
	}
	else
	{
		m_installedMenus.AddTail(pMenu);
	}
	pMenu->SetBlock( this );

	luAt = luBegin + GetCount() - pMenu->GetTrailingReserved();
	int sCount = (int)pMenu->GetCount();
	for ( chint16 i = 0; i < sCount; i++)
	{
		ChMenuItem *pItem = pMenu->FindItem(i);
		InstallItem(pItem, &luAt, boolToHead, pNatMenu);
	}
	return (pMenu);
}

void ChRMenuBlock::InstallItem(ChMenuItem *pItem, chuint32 *pluAt,
	chuint16 suWhere, ChNativeMenu * pNatMenu)
{
	ChRMenuItem**	ppRItem;
	ChRMenuItem*	pRItem;
	chuint32		luAt = 0;

	if (pluAt)
	{
		luAt = *pluAt;
	}
	else
	{
		m_pContainedIn->FindBlock( m_strModuleName, &luAt );
		luAt += GetCount();
	}

	if (!pNatMenu)
	{
		pNatMenu = m_pContainedIn->GetNative();
	}

	switch(pItem->GetType())
	{
		case ChMenuItem::CH_MENU_ITEM_TEXT:
		{
			if (ppRItem = m_itemNames.Find( pItem->GetTitle() ))
			{
				pRItem = *ppRItem;
			}
			else
			{								/* ChRMenuItem is not found, so
												make it and add it */

				ChMenuItemId	id = GetRMenu()->GetMenuMgr()->MakeId();

				pRItem = new ChRMenuItem( this, id );
				m_itemNames.Insert( pItem->GetTitle(), pRItem );
				GetRMenu()->GetMenuMgr()->PutItem( id, pRItem );

				#if defined( CH_MSW )
				{							// Now add the item to the hMenu

					UINT	uFlags = MF_BYPOSITION;
				
					uFlags |= pItem->IsEnabled() ? MF_ENABLED : MF_GRAYED;
					uFlags |= pItem->IsChecked() ? MF_CHECKED : MF_UNCHECKED;
				
					pNatMenu->InsertMenu( (UINT)luAt, uFlags, (UINT)id,
											pItem->GetDisplayTitle() );
					pNatMenu->EnableMenuItem( (UINT)luAt, uFlags );
				}
				#elif defined( CH_UNIX )
				{
					Widget pulldown, button;

					XtVaGetValues( *pNatMenu, XmNsubMenuId, &pulldown, NULL);
					button = XtVaCreateManagedWidget( pItem->GetDisplayTitle(),
													  xmPushButtonGadgetClass,
													  pulldown, NULL );
					XtSetSensitive( button, pItem->IsEnabled() );
					// XXX Note that we don't handle IsChecked()
					XtAddCallback( button, XmNactivateCallback, ChRMenuItem_cb,
								   (XtPointer) pRItem );
					pRItem->SetWidget( button );
				}
				#endif	// defined( CH_UNIX )

				luAt++;
			}
		 	break;
		}

		case ChMenuItem::CH_MENU_ITEM_CASCADE:
		{
			if (ppRItem = m_itemNames.Find( pItem->GetTitle() ))
			{
				pRItem = *ppRItem;
			}
			else
			{								/* ChRMenuItem is not found, so
												make it and add it */

				ChMenuItemId	id = GetRMenu()->GetMenuMgr()->MakeId();
				ChRMenu*		pChildRMenu;

				pRItem = new ChRMenuItem( this, id );

				m_itemNames.Insert( pItem->GetTitle(), pRItem );
				GetRMenu()->GetMenuMgr()->PutItem( id, pRItem );

											/* Now make a new child ChRMenu,
												and install the child ChMenu */

				pChildRMenu = new ChRMenu( GetRMenu()->GetMenuMgr(), pItem->GetTitle(), pRItem );
				pChildRMenu->Install( pItem->GetMenu(), m_strModuleName,
										suWhere != CH_MENU_AT_TAIL );

											// Now add the item to the hMenu
				#if defined( CH_MSW )
				{
					ChNativeMenuHdl		hChildMenu;
					UINT				uFlags = MF_BYPOSITION | MF_POPUP;

					hChildMenu = pChildRMenu->GetNativeHdl();

					uFlags |= pItem->IsEnabled() ? MF_ENABLED : MF_GRAYED;
					uFlags |= pItem->IsChecked() ? MF_CHECKED : MF_UNCHECKED;

					pNatMenu->InsertMenu( (UINT)luAt, uFlags, (UINT)hChildMenu,
											pItem->GetDisplayTitle() );
					pNatMenu->EnableMenuItem( (UINT)luAt, uFlags );
					luAt++;
				}
				#else	// defined( CH_MSW )
				{
					NOT_IMPLEMENTED;
				}
				#endif	// defined( CH_MSW )
			}
		 	break;
		}

		case ChMenuItem::CH_MENU_ITEM_SEPARATOR:
		{
			ChMenuItemId	id;

			id = GetRMenu()->GetMenuMgr()->MakeId();
			pRItem = new ChRMenuItem( this, id );

			m_itemNames.Insert( MakeSeparatorTitle( id ), pRItem );
			GetRMenu()->GetMenuMgr()->PutItem( id, pRItem );

			#if defined( CH_MSW )
			{								// Now add the item to the hMenu

				UINT	uFlags = MF_BYPOSITION | MF_SEPARATOR;
			
				pNatMenu->InsertMenu( (UINT)luAt, uFlags );
			}
			#elif defined( CH_UNIX )
			{
				Widget pulldown, button;

				XtVaGetValues( *pNatMenu, XmNsubMenuId, &pulldown, NULL);
				button = XtVaCreateManagedWidget( pItem->GetDisplayTitle(),
												  xmSeparatorGadgetClass,
												  pulldown, NULL );
			}
			#endif	// defined( CH_UNIX )

			luAt++;
			break;
		}

		default:
		{
			break;
		}
	}
											// Now add the item to the RItem
	ChMenuSplay*	pSplayAfter = 0;

	if (suWhere == CH_MENU_AT_EXISTING)
	{
		ChPosition pos = m_installedMenus.GetHeadPosition();
		ChMenu *pMenu, *pParentMenu = pItem->GetParent();
		pSplayAfter = new ChMenuSplay;
		while( pos != NULL )
		{
			pMenu = m_installedMenus.GetNext(pos);
			if (pParentMenu == pMenu) break;
		}
		while( pos != NULL )
		{
			pMenu = m_installedMenus.GetNext(pos);
			pSplayAfter->Insert(pMenu, 0);		 // just using as bag
		}
	}

	pRItem->Add( pItem, suWhere, pSplayAfter );
	delete pSplayAfter;

	if (pluAt)
	{
		*pluAt = luAt;
	}
	pRItem->AddAccelerator( pItem );
}

// load both the RMenuBlock and the pMenu from the native menu, preserving ids

ChMenu* ChRMenuBlock::Init(ChNativeMenu * pNatMenu, ChMenu *pMenu, bool boolToHead )
{
#ifdef CH_MSW
	// Walk the menu; add each item to the ChMenu,
	// and the RItem to the ChRMenuBlock
	chint16 sCount = pNatMenu->GetMenuItemCount();
	LPTSTR pBuf = new TCHAR[MAX_MENU_NAME + 1];
	for (int i = 0; i < sCount; i++ )
	{

		//CMenu *pMenu =  pBar->GetSubMenu(i);
		UINT id = pNatMenu->GetMenuItemID( i );

		switch((int)id)
		{
			case -1:
			{
				// it's a submenu
				NOT_IMPLEMENTED;
				break;
			}
			case 0:
			{
				// separator
				pMenu->InsertSeparator( );

				// ChRMenuItem is a nameless, idless separator, so make it and add it
				ChMenuItemId id = GetRMenu()->GetMenuMgr()->MakeId();
				ChRMenuItem *pRItem = new ChRMenuItem( this, id );
				pRItem->Add(pMenu->FindItem(i));

				m_itemNames.Insert(MakeSeparatorTitle(id), pRItem);
				GetRMenu()->GetMenuMgr()->PutItem(id, pRItem);

				break;
			}

			default:
			{								/* string or ownerdraw item (we
												only handle text) */
				pNatMenu->GetMenuString( i, pBuf, MAX_MENU_NAME, MF_BYPOSITION );
				pMenu->InsertItem( ChRMenuItem::SplitDisplayTitle(pBuf) );

				ChRMenuItem*	pRItem = new ChRMenuItem( this, id );

				pRItem->Add( pMenu->FindItem( i ) );
				m_itemNames.Insert( pRItem->GetTitle(), pRItem );
				GetRMenu()->GetMenuMgr()->PutItem( id, pRItem );
				break;
			}
		}
	}

	delete []pBuf;

	return pMenu;
#else
	NOT_IMPLEMENTED;
	return 0;
#endif
}

bool ChRMenuBlock::Uninstall( ChMenu *pMenu )
{
	// !!! This is too inefficient; it ought to loop thru pMenu's items, calling
	// Delete(pItem) for each RItem backpointer.
	ChPosition pos = m_installedMenus.Find(pMenu);
	if (pos)
	{
		m_installedMenus.Remove( pos );		// delete from pMenu from list
		#if !defined( NO_TEMPLATES )
		ChList<ChString> deadList;    
		#else 
		ChStrList	   deadList;
		#endif
											// delete items from their ritem lists
		ChDeleteItemOp	zapMenu( pMenu, &deadList );

		m_itemNames.Infix( zapMenu );

		// Now we need to delete the ritems that the ChDeleteItemOp marked for deletion
		// by recording their keys in the deadList; we need to keep deadlist
		// because we can't delete items during infix walk
		ChString deadKey;
		while( !deadList.IsEmpty() )
		{
			deadKey = deadList.RemoveHead();
			m_itemNames.Delete( deadKey );
		}
	}

	pMenu->SetBlock( 0 );

	return false;
}


#if defined( CH_ARCH_16 )
chuint32 ChRMenuBlock::GetCount() 
#else
chuint32 ChRMenuBlock::GetCount() const
#endif
{
	ChMyCountOp		countOp;

	m_itemNames.Infix( countOp );

	return countOp.GetCount();
}


void ChRMenuBlock::Promote( ChMenu *pMenu, bool boolToHead )
{
											/* Move the menu to the requested
												location */

	ChPosition	pos = m_installedMenus.Find( pMenu );

	if (pos)
	{
		m_installedMenus.Remove( pos );
	}

	if(boolToHead)
	{
		m_installedMenus.AddHead( pMenu );
	}
	else
	{
		m_installedMenus.AddTail( pMenu );
	}
	// Now do it for the items
	int sCount = (int)pMenu->GetCount();
	for ( chint16 i = 0; i < sCount; i++)
	{
		ChMenuItem *pItem = pMenu->FindItem(i);
		ChRMenuItem *pRItem = pItem->GetRItem();  // ain't backpointers wunnerful?
		// Now promote the item within the RItem
		pRItem->Promote(pItem, boolToHead);
	}

	return;
}

void ChRMenuBlock::DeleteItem( ChRMenuItem *pRItem, ChMenu *pMenu, bool boolRemove )
{
	ChPosition	pos = pRItem->m_items.GetHeadPosition(  );
	ChPosition	oldPos = pos;
	ChMenuItem*	pItem = 0;
	
	while( pos != NULL ) {
		pItem = pRItem->m_items.GetNext( pos );
		if (pItem->GetParent() == pMenu) {
			DeleteItem( pRItem, pItem, boolRemove );
			break;
		}
		oldPos = pos;
	}
}


void ChRMenuBlock::DeleteItem( ChRMenuItem *pRItem, ChMenuItem *pMenuItem, bool boolRemove )
{
	ChString	strTitle = pRItem->GetTitle();	  // need to save these for later
#ifdef CH_MSW
	ChMenuItem::ItemType myType = pRItem->GetType();
#endif

	pRItem->Delete( pMenuItem );
	if (pRItem->GetCount() == 0)
	{
		// no longer needed
		#if defined(CH_MSW)
		// first zap the real thing, in MSW-land
		ChNativeMenu * pNatMenu = m_pContainedIn->GetNative();
		switch (myType)
		{
			case ChMenuItem::CH_MENU_ITEM_TEXT:
			{
				pNatMenu->DeleteMenu((UINT)pRItem->GetId(), MF_BYCOMMAND);
				break;
			}
			case ChMenuItem::CH_MENU_ITEM_CASCADE:
			{

				UINT	i, uCount = pNatMenu->GetMenuItemCount();
				ChMenu * pCascadeMenu = pMenuItem->GetMenu();
				ChRMenu * pRCascadeMenu = pCascadeMenu->GetBlock()
					->m_pContainedIn;
				HMENU hCascadeNatMenu = pRCascadeMenu->GetNativeHdl();
				for (i = 0; i < uCount; i++)
				{
					HMENU hMenu = pNatMenu->GetSubMenu(i)->GetSafeHmenu();
					if (hMenu == hCascadeNatMenu)
					{
						pNatMenu->RemoveMenu(i, MF_BYPOSITION);
						break;
					}
				}
				if(pRCascadeMenu->Uninstall(pCascadeMenu, pCascadeMenu->GetBlock()))
					delete pRCascadeMenu;


				break;
			}
			case ChMenuItem::CH_MENU_ITEM_SEPARATOR:
			{
				//pNatMenu->DeleteMenu((UINT)pRItem->GetId(), MF_BYCOMMAND);
				// CAN'T DELETE By location, so just do a cleanup
				strTitle = MakeSeparatorTitle(pRItem->GetId());
				break;
			}
			default:
			{
				// leak??
				break;
			}
		}
		#else
			NOT_IMPLEMENTED;
		#endif	// CH_MSW
		// Now zap the RItem, and remove all traces of it
		GetRMenu()->GetMenuMgr()->DeleteItem(pRItem->GetId());	// depends on id still valid!
		if( boolRemove)
		{
			m_itemNames.Delete(strTitle);	// told you we'd need this; title() is now invalid!
			delete pRItem;
		}

	}
}

void ChRMenuBlock::AddAccelerator( ChMenuItemId id, const chuint32 luChar, const chflag16 fsMods )
{
#ifdef CH_MSW
	LPACCEL	pAccel;
	int count = 0;

	if (m_hAccelTable)
	{
		count = ::CopyAcceleratorTable(m_hAccelTable, 0, 0);
		pAccel = new ACCEL[count+1];
		::CopyAcceleratorTable(m_hAccelTable, pAccel, count);
		// test to see if already there
		for (int j = 0; j < count; j++)
		{
			if (pAccel[j].cmd == (WORD)id)	// should we allow more than one per id?? only one would be on 
										// menu display.
			{
				delete [] pAccel;
				return;
			}
		}
	}
	else
	{
		pAccel = new ACCEL[count+1];
	}
	pAccel[count].fVirt = (BYTE)fsMods;
	pAccel[count].key =   (WORD)luChar;
	pAccel[count].cmd =   (WORD)id;

	HACCEL hTmp;
	if (hTmp = ::CreateAcceleratorTable(pAccel, count + 1))
	{
		if (m_hAccelTable)
		{
			GetRMenu()->GetMenuMgr()->UnregisterAcceleratorTable(m_hAccelTable);
			::DestroyAcceleratorTable( m_hAccelTable );
		}
	 	m_hAccelTable = hTmp;
		GetRMenu()->GetMenuMgr()->RegisterAcceleratorTable(m_hAccelTable);
	}
	else
	{
		TRACE0("Failed to create accel table!");
	}
	delete [] pAccel;
#else
	NOT_IMPLEMENTED;
#endif
	return;
}

void ChRMenuBlock::DeleteAccelerator( ChMenuItemId id )
{
#ifdef CH_MSW
	LPACCEL	pAccel;
	int count, newCount;

	if (m_hAccelTable)
	{
		newCount = count = ::CopyAcceleratorTable(m_hAccelTable, 0, 0);
		pAccel = new ACCEL[count];
		::CopyAcceleratorTable(m_hAccelTable, pAccel, count);
		// test to see if  there, move tail
		for (int j = 0; j < count; j++)
		{
			if (pAccel[j].cmd == (WORD)id)	
			{
				if ( j != count - 1)
				{
					// move tail if any
					memmove(pAccel + j, pAccel + j + 1, (count - j - 1) * sizeof(ACCEL));
				}
				newCount--;
				break;
			}
		}
		if (newCount != count)
		{
			if( newCount )
			{
				HACCEL hTmp;
				if (hTmp = ::CreateAcceleratorTable(pAccel, newCount))
				{
					GetRMenu()->GetMenuMgr()->UnregisterAcceleratorTable(m_hAccelTable);
					::DestroyAcceleratorTable( m_hAccelTable );
				 	m_hAccelTable = hTmp;
					GetRMenu()->GetMenuMgr()->RegisterAcceleratorTable(m_hAccelTable);
				}
				else
				{
					TRACE0("Failed to create accel table!");
				}
			}
			else
			{
				// count went to zero, zap it
				GetRMenu()->GetMenuMgr()->UnregisterAcceleratorTable(m_hAccelTable);
				::DestroyAcceleratorTable( m_hAccelTable );
			 	m_hAccelTable = 0;
			}
		}
		delete [] pAccel;
	}
#else
	NOT_IMPLEMENTED;
#endif
}

/*----------------------------------------------------------------------------
	ChRMenuBlock private or protected helper methods
----------------------------------------------------------------------------*/

ChString ChRMenuBlock::MakeSeparatorTitle( ChMenuItemId id ) const
{        
	#if 1
	//#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	// using ostrstream under win16 causes memory leak, so we do
	// this the hard way
	char strTitle[100];
	::wsprintf( strTitle, "Ch!!Separator %ld", (chuint32)id );
	return ChString( strTitle );
	#else
	ostrstream sTitle;

	sTitle << "Ch!!Separator " << id << flush;
	return (sTitle.str());
	#endif
}  

// Accelarator create, copy destroy functions are available only on win32 
// we need to provide the same functionality for win16
#if defined( CH_MSW ) && defined( CH_ARCH_16 )    
CH_GLOBAL_FUNC( HACCEL )
CreateAcceleratorTable( LPACCEL pAccel, int cEntries )
{               
	TRACE( "CreateAcceleratorTable table not implemented for win16" );
	return NULL;
}

CH_GLOBAL_FUNC( int )
CopyAcceleratorTable( HACCEL hAccel, LPACCEL pAccel, int cEntries )
{               
	TRACE( "CopyAcceleratorTable table not implemented for win16" );
	return true;
}


CH_GLOBAL_FUNC( BOOL )
DestroyAcceleratorTable( HACCEL hAccel )
{               
	TRACE( "DestroyAcceleratorTable table not implemented for win16" );
	return true;
}   

#endif

// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
