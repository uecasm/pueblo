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
	
		ChRMenu
		ChRMenuItem
		ChRMenuMgr

	This file also contains module interface definitions and should be
	included by every module.

----------------------------------------------------------------------------*/

// $Header$

#if (!defined( CHRMENU_H ))
#define CHRMENU_H

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif

#include <iostream>
#if	defined( CH_MSW ) && defined( WIN32 )
#include <afxtempl.h>
#endif
		  // afxcoll.h
#include <ChTypes.h>
#include <ChVers.h>
#include <ChMsg.h>
#include <ChDispat.h>
#include <ChMenu.h>

#if defined( NO_TEMPLATES )
#include <TemplCls\ChMnItem.h>
#include <TemplCls\ChMnMap.h>
#include <TemplCls\ChMnList.h>
#include <TemplCls\ChMnList.inl>
#include <TemplCls\ChMnBlk.h>
#include <TemplCls\ChMnBlk.inl>
#include <TemplCls\ChMItSpl.h>
#endif


#if !defined(CH_MSW)
#define HACCEL	int
#endif

/*============================================================================
	Menu constants
============================================================================*/

#define CH_MENU_FIRST_ID	10000
#define CH_MENU_LAST_ID		15000


#define CH_MENU_AT_HEAD			0x01
#define CH_MENU_AT_TAIL			0x02
#define CH_MENU_AT_EXISTING		0x04

/*============================================================================
	Forward Menu Class decls
============================================================================*/

typedef chuint32 ChMenuItemId;
class ChInstalledMenu;
class ChRMenuItem;
typedef ChRMenuItem *pChRMenuItem;
typedef ChRMenu *pChRMenu;
class ChRMenuBlock;
typedef ChMenu * pChMenu;  

class ChRMenuMgr;

#if !defined( NO_TEMPLATES ) 

typedef ChSplay<pChMenu, chparam> t_ChMenuSplay;

#else

#include <TemplCls\ChMnSply.h>
#include <TemplCls\ChMnItLs.h>
#include <TemplCls\ChMnItLs.inl>
#include <TemplCls\ChStrLst.h>
#include <TemplCls\ChStrLst.inl>

#endif

class ChMenuSplay : public t_ChMenuSplay { };

/*============================================================================
	ChRMenu class
============================================================================*/

class ChRMenu 
{
	protected:
	#if !defined( NO_TEMPLATES )
		ChPtrList<ChRMenuBlock>	m_MenuBlocks;	// need list of lists 
												// one block per mod name
												// and one for shared      
	#else
		ChMenuBlockPtrList		m_MenuBlocks;	 
	#endif
		ChRMenu*				m_pParent;
		ChNativeMenuHdl			m_hMenu;
		bool					m_boolKeepHMenu;
		ChMenu*					m_pMyMenu;		// Menu I created at init time
		ChRMenuItem*			m_pRItem;		// for cascades; contained in this item
		ChString					m_strTitle;		// used only for destruction!
		ChRMenuMgr* 			m_pMenuMgr;		// Menu manager for this menu block

	public:
		
		ChRMenu( ChRMenuMgr* pMenuMgr, const ChString& strTitle, ChNativeMenu * pMenu, const ChMsgHandler pHandler );
		ChRMenu( ChRMenuMgr* pMenuMgr, const ChString& strTitle, ChNativeMenu * pMenuBar );
		ChRMenu( ChRMenuMgr* pMenuMgr, const ChString& strTitle, ChRMenuItem *pItem  );

		virtual ~ChRMenu();

		inline ChRMenuMgr* GetMenuMgr() const		{ return  m_pMenuMgr; }

		ChMenu* Install( ChMenu *pMenu, const ChString& strModuleName, bool boolToHead = true);
		void Promote(ChMenu *pMenu, bool boolToHead = true);
		inline ChNativeMenuHdl GetNativeHdl() { return m_hMenu; };
		#if defined(CH_MSW)
		inline ChNativeMenu * GetNative() { return CMenu::FromHandle(m_hMenu); };  // temporary object
		#else
		ChNativeMenu *GetNative() { return &m_hMenu; };
		#endif // CH_MSW
		bool Uninstall( ChMenu *pMenu, ChRMenuBlock *pBlock );	// true return=>delete me
		ChRMenuBlock* FindBlock(ChString strModName, chuint32 *pluBegin = 0);
		inline bool ShouldKeepMenu() { return m_boolKeepHMenu; };
		inline void SetKeepMenu( bool boolShouldKeep ) { m_boolKeepHMenu = boolShouldKeep; };

	protected:
		ChRMenu* CleanupSeparators();


};

class ChRMenuBlock;

class ChRMenuItem 
{
	friend class ChRMenuBlock;

	public:
		ChRMenuItem( ChRMenuBlock *pBlock, ChMenuItemId id = 0 );

		virtual ~ChRMenuItem();

		chint32 GetCount();
		ChMenuItemId GetId() { return m_id; }
		inline ChRMenuBlock *GetBlock() const { return m_pBlock; }
		ChRMenuMgr* GetMenuMgr() const;


											// these set/get common data
		void Enable( bool boolEnable = true );
		bool IsEnabled();
		void Check( bool boolCheck = true );
		bool IsChecked();
		ChString& GetTitle();
		inline ChString& GetDisplayTitle() { return m_strDisplayTitle; };
		ChMenuItem::ItemType GetType();
											// these return data for foremost
		ChString& GetHelpText() ;
		ChMenu *GetMenu() ;

											// Item/item list manipulators

		void Add( ChMenuItem *pMenuItem, bool boolToHead = true );
		void Add( ChMenuItem *pMenuItem, chuint16 suWhere,
					ChMenuSplay *pSplayAfter );
		void Promote( ChMenu *pMenu, bool boolToHead = true );
		void Promote( ChMenuItem *pMenuItem, bool boolToHead = true );
		void Delete( ChMenu *pMenu );
		void Delete( ChMenuItem *pMenuItem );
		ChMenuItem* GetItem( chint16 sIndex );

											// Notification

		void Notify( ChMenuMsg& msg, bool boolStopOnProcessed = true,
						bool* pboolEnabled = 0 );
		void NotifyChanged();


		static ChString  SplitDisplayTitle(const ChString & strDisplayTitle );
		void AddAccelerator(const ChMenuItem * pItem);

#ifdef CH_UNIX
    Widget GetWidget(void) { return m_widget; };
    void SetWidget(Widget w) { m_widget = w; };
#endif

	protected:
		ChMenuItemId			m_id;		// Native item identifier
	#if !defined( NO_TEMPLATES )
		ChPtrList<ChMenuItem>	m_items;	// list of items to be notified 
	#else
		ChMenuItemList			m_items;
	#endif
		ChRMenuBlock*			m_pBlock;	// Container block
		ChString					m_strDisplayTitle;
		chflag16				m_fsAccMods;	// The -first- accel ever set for this ritem
		chuint32				m_luAccChar;	// Accel is kept for life of ritem
		ChString  MakeDisplayTitle( );

#ifdef CH_UNIX
		Widget m_widget;
#endif
 
};

class ChRMenuBlock 
{
	public:
		
		ChRMenuBlock( ChString strModName, ChRMenu *pRMenu  );

		virtual ~ChRMenuBlock();

		ChMenu* Install( ChNativeMenu* pNatMenu, ChMenu* pMenu,
							chuint32 luBegin, bool boolToHead = true );
		ChMenu* Init( ChNativeMenu* pNatMenu, ChMenu* pMenu,
							bool boolToHead = true );
		bool Uninstall( ChMenu *pMenu );
		void InstallItem( ChMenuItem* pItem, chuint32* pluAt,
							chuint16 suWhere = CH_MENU_AT_HEAD,
							ChNativeMenu * pNatMenu = 0 );

		//void InsertItem(ChMenuItem *pItem, chuint32 *pluAt, bool boolToHead, ChNativeMenu * pNatMenu = 0);
        #if defined( CH_ARCH_16 )
		chuint32 GetCount();
		#else
		chuint32 GetCount() const;
		#endif
		ChString& GetModuleName() { return m_strModuleName; }
		void Promote( ChMenu *pMenu, bool boolToHead = true );
		void DeleteItem( ChRMenuItem *pRItem, ChMenu *pMenu, bool boolRemove = true );
		void DeleteItem( ChRMenuItem *pRItem, ChMenuItem *pMenuItem, bool boolRemove = true );
		ChString MakeSeparatorTitle( ChMenuItemId id ) const;
		inline ChRMenu *GetRMenu() { return m_pContainedIn; }
		void AddAccelerator( ChMenuItemId id, const chuint32 luChar, const chflag16 fsMods );
		void DeleteAccelerator( ChMenuItemId id );
		//void AddAccelerator( ChRMenuItem *pItem );
		//void DeleteAccelerator( ChRMenuItem *pItem );

	protected:
	#if !defined( NO_TEMPLATES )
		ChPtrList<ChMenu>				m_installedMenus;  
    #else
    	ChMenuPtrList                   m_installedMenus;
	#endif
			
											/* The following is the unique key
												for block, within the ChRMenu */

		ChString							m_strModuleName;

		#if !defined( NO_TEMPLATES )
		ChSplay<ChString, ChRMenuItem *>	m_itemNames;	// RItems are keyed by itemname	  
		#else
		ChMenuItemSplay					m_itemNames;   
		#endif
		ChRMenu*						m_pContainedIn;
		HACCEL							m_hAccelTable;	 // Accelerator table

											// ChRMenuBlock item Iterators

		class ChDeleteBlocksOp : public 
								#if !defined( NO_TEMPLATES )
								ChVisitor2<ChString, ChRMenuItem*>  
								#else 
								ChMenuItemSplayVisitor2
								#endif
		{
			public:
				bool Visit( const ChString& key, const pChRMenuItem& pBlock )
						{
							delete pBlock;
							return true;
						}
		};

		class ChMyCountOp : public 
								#if !defined( NO_TEMPLATES )
								ChVisitor2<ChString, ChRMenuItem*>  
								#else 
								ChMenuItemSplayVisitor2
								#endif
		{
		  protected:
		  	chuint32 m_luCount;
		  public:
			inline chuint32 GetCount() { return m_luCount; };
			void Start() { m_luCount = 0; };
			bool Visit(const ChString& key, const pChRMenuItem& data){ 	m_luCount++;
																		return true;};
		};

		class ChDeleteItemOp : public 
								#if !defined( NO_TEMPLATES )
								ChVisitor2<ChString, ChRMenuItem*>  
								#else 
								ChMenuItemSplayVisitor2
								#endif
		{
			public:
				#if !defined( NO_TEMPLATES )
				ChDeleteItemOp( ChMenu *pMenu, ChList<ChString> *pDeadList )
				 : m_pMenu( pMenu ), m_pDeadList(pDeadList) { }
				#else
				ChDeleteItemOp( ChMenu *pMenu, ChStrList *pDeadList )
				 : m_pMenu( pMenu ), m_pDeadList(pDeadList) { }
				#endif

				bool Visit( const ChString& key, const pChRMenuItem& pRMenu )
						{
							#if 0
							pRMenu->Delete( m_pMenu );
							if (pRMenu->GetCount() == 0)
							{
								m_pDeadList->AddHead( key );
								delete pRMenu;
							}
							#endif
							ChRMenuBlock *parent = pRMenu->GetBlock();
							parent->DeleteItem( pRMenu, m_pMenu, false);
							if (pRMenu->GetCount() == 0)
							{
								m_pDeadList->AddHead( key );
								delete pRMenu;
							}
							return true;
						}

			protected:
				ChMenu *m_pMenu;
				#if !defined( NO_TEMPLATES )
				ChList<ChString> *m_pDeadList; 
				#else
				ChStrList	   *m_pDeadList;	
				#endif
		};  
};

class ChCore;

class CH_EXPORT_CLASS ChRMenuMgr			// exactly one of these per client app
{
	protected:
											// Main mapping collections
		#if !defined( NO_TEMPLATES )
		ChSplay<ChString, ChRMenu*>	m_menus;
		ChPtrSplay<ChRMenuItem>		m_itemTree;
		#else
		ChMenuMapSplay				m_menus;   
		ChMenuItemPtrSplay			m_itemTree;
		#endif
		ChMenuItemId				m_nextId;
		ChNativeMenuHdl				m_hBar;	// probably redundant
	//	static ChRMenuMgr*			m_pTheMgr;
		ChCore*						m_pCore;
		ChParamList					m_accelerators;	  	// it's -much- more efficient
														// to keep this than walking blocks 

		class ChDeleteRMenuOp : public 
						#if !defined( NO_TEMPLATES )
						ChVisitor2<ChString, ChRMenu*>
						#else
						ChMenuMapSplayVisitor2
						#endif
		{
			public:
				bool Visit( const ChString& key, const pChRMenu& data )
						{
							data->SetKeepMenu(true);
							delete data;
							return true;
						}
		}; 

	public:
		ChRMenuMgr( ChCore* pCore );
		
		virtual ~ChRMenuMgr();

		ChCore*	GetCore() const 			{ return m_pCore; }

		// Finds the right ChRMenu, then puts it in; adds elements
		// to ChRMenuItem lists
		ChMenu* Install( ChMenu *pMenu, const ChModuleID idModule, bool boolToHead = true);
		ChMenu* Install( ChMenu *pMenu, const ChString& strModuleName, bool boolToHead = true);

		// Initialize, absorbing the existing menuBar, and assocating it with some module
		void Init( ChNativeMenu *pBar, const ChModuleID idModule, 
			const ChMsgHandler pHandler,  const HACCEL m_hAccelTable );
		ChMenuItemId	MakeId();
		ChRMenuItem *GetItem(ChMenuItemId id);
		void DeleteItem(ChMenuItemId id);
		ChRMenuItem *PutItem(ChMenuItemId id, ChRMenuItem *pRItem);
		ChRMenu *GetRMenu(ChString );
		//static ChRMenuMgr *GetMgr( );
		inline ChNativeMenuHdl GetBarHandle() { return m_hBar;	};
		bool Notify( ChMenuItemId id, chuint32 uMessage = CH_MSG_MENU_SELECT,
						bool* pboolEnabled = 0 );
		bool GetMessageString(UINT nID, ChString& rMessage);

		void RemoveRMenu(ChString& name ) { m_menus.Delete( name ); };

		#if defined(CH_MSW)
		void RegisterAcceleratorTable( HACCEL hAccel );
		void UnregisterAcceleratorTable( HACCEL hAccel );
		int TranslateAccelerators( HWND hwnd, LPMSG lpmsg );
		#endif // CH_MSW

};

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR    
#endif

#endif // CHRMENU_H

// $Log$
