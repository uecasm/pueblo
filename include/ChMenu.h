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

#if (!defined( CHMENU_H ))
#define CHMENU_H

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif

#include <iostream>

#if defined( CH_MSW )  && !defined( NO_TEMPLATES )

	#include <afxtempl.h>

#endif

#include <ChTypes.h>
#include <ChVers.h>
#include <ChMsg.h>
#include <ChDispat.h>
#include <ChExcept.h>
#include <ChList.h>


/*----------------------------------------------------------------------------
	Menu constants
----------------------------------------------------------------------------*/

#define CH_MENU_END				(-1)
#define CH_MENU_OPT_DELIMITED	(1)

#define CH_STATE_ENABLED		((chuint32)1)
#define	CH_STATE_CHECKED		((chuint32)2)

#if defined( CH_MSW )

#if defined( CH_ARCH_16 )  && !defined( FVIRTKEY )
#define FVIRTKEY  TRUE          /* Assumed to be == TRUE */
#define FNOINVERT 0x02
#define FSHIFT    0x04
#define FCONTROL  0x08
#define FALT      0x10
#endif

#define CH_ACC_ALT			FALT
#define CH_ACC_CONTROL		FCONTROL
#define CH_ACC_NOINVERT		FNOINVERT
#define CH_ACC_SHIFT		FSHIFT
#define CH_ACC_VIRTKEY		0x01
//#define CH_ACC_VIRTKEY		FVIRTKEY
#else
#define CH_ACC_ALT			0x01
#define CH_ACC_CONTROL		0x02
#define CH_ACC_NOINVERT		0x04
#define CH_ACC_SHIFT		0x08
#define CH_ACC_VIRTKEY		0x10
#endif


/*----------------------------------------------------------------------------
	Menu identifiers
----------------------------------------------------------------------------*/

#ifdef CH_UNIX
#define ID_HELP_INDEX	1
#define ID_EDIT_CUT		2
#define ID_EDIT_COPY	3
#define ID_EDIT_PASTE	4
#define ID_APP_EXIT		6
#endif

#if defined(CH_MSW) && defined( CH_ARCH_16 )

typedef struct tagACCEL {
    BYTE   fVirt;               /* Also called the flags field */
    WORD   key;
    WORD   cmd;
} ACCEL, *LPACCEL;


CH_EXTERN_FUNC( HACCEL )
CreateAcceleratorTable( LPACCEL pAccel, int cEntries );
CH_EXTERN_FUNC( int )
CopyAcceleratorTable( HACCEL hAccel, LPACCEL pAccel, int cEntries );
CH_EXTERN_FUNC( BOOL )
DestroyAcceleratorTable( HACCEL hAccel );
#endif


/*----------------------------------------------------------------------------
	Helper Menu classes, and forward declarations
----------------------------------------------------------------------------*/

class ChMenuItem;
typedef ChMenuItem *pChMenuItem;
class ChMenuMsg;
class ChRMenu;
class ChRMenuItem;
class ChRMenuBlock;
class ChRMenuMgr;        

#if defined( NO_TEMPLATES )
#include <TemplCls\ChMnPtrL.h>
#include <TemplCls\ChMnPtrL.inl>
#endif


/*----------------------------------------------------------------------------
	Native Menu types
----------------------------------------------------------------------------*/

#if defined(CH_MSW)
typedef CMenu	ChNativeMenu;	   // we use objects for most apis
typedef HMENU	ChNativeMenuHdl;   // objects are temporary in MFC,
								   // so we store handles
#elif defined(CH_UNIX)
typedef Widget	ChNativeMenu;
typedef Widget  ChNativeMenuHdl;
#else
typedef int	ChNativeMenu;
typedef int	ChNativeMenuHdl;
#endif


class ChRMenuMgr;

/*----------------------------------------------------------------------------
	ChMenuEx class
----------------------------------------------------------------------------*/

#if defined( CH_EXCEPTIONS )

class CH_EXPORT_CLASS ChMenuEx : public ChEx
{
	public:
		ChMenuEx() : ChEx() {}
		virtual ~ChMenuEx() {}
};

#endif	// defined( CH_EXCEPTIONS )


/*----------------------------------------------------------------------------
	ChMenu class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChMenu				//  Should inherit from ChStreamable
{
	friend class ChRMenuBlock;
	friend class ChRMenu;
	friend class ChMenuItem;

	public:
		
		ChMenu( ChRMenuMgr* pMgr, ChString strTitle, const ChMsgHandler pDefHandler,
				chflag32 flOptions = 0  );
		virtual ~ChMenu();
		
		inline ChRMenuMgr* GetMenuMgr() const { return m_pMenuMgr; }

		inline chint32 GetCount() { return m_items.GetCount(); }
		inline ChMsgHandler GetHandler() { return m_pHandler; }
		inline ChString& GetTitle() { return m_strTitle; }
		inline bool IsDelimited()
						{
							return 0 != (m_flOptions & CH_MENU_OPT_DELIMITED);
						}
		inline bool IsEnabled() const
						{
							return 0 != (m_flState &  CH_STATE_ENABLED);
						}
		inline bool IsInstalled() { return m_pBlock != 0; }

		ChMenuItem* InsertItem( ChString strTitle, const ChMsgHandler pHandler = 0,
								chint16 sPosition = CH_MENU_END);
		ChMenuItem* InsertItem( ChMenu *pCascadeMenu,
								chint16 sPosition = CH_MENU_END);
		ChMenuItem* InsertSeparator( chint16 sPosition = CH_MENU_END );
		ChMenuItem* FindItem( ChString strTitle, chint16 sPosition = 0 );
		ChMenuItem* FindItem( ChMenu *pMenu, chint16 sPosition = 0 );
		ChMenuItem* FindItem( chint16 sPosition = 0 );
		void Promote( bool boolToHead = true );
		void Enable( bool boolEnable = true );
		ChMenu* Install( const ChModuleID idModule, bool boolToHead = true );
		ChMenu* Install( const ChString& strModuleName, bool boolToHead = true );
		void Uninstall();
		void SetHandler( const ChMsgHandler pHandler = 0 );
		void SetTitle( ChString strTitle );

		chint16 ChMenu::GetItemIndex(ChMenuItem*  pItem); // oughta be protected
 		virtual chint16 GetTrailingReserved() { return 0; };

	protected:
		ChMenuItem* InsertInList( ChMenuItem* pItem, chint16 sPosition = CH_MENU_END);
		ChPosition FindItemPosition( chint16 sPosition );
		inline void SetBlock(ChRMenuBlock *pBlock) { m_pBlock = pBlock; };
		inline ChRMenuBlock *GetBlock() { return (m_pBlock); };
		inline ChMsgHandler *GetHandlerAddr()  {return &m_pHandler;};
		ChMenuItem* ChMenu::FindItemByID(chuint32 id);
		void RemoveItem(ChMenuItem* pItem);

	protected:     
		#if !defined( NO_TEMPLATES )
		ChPtrList<ChMenuItem>	m_items;    
		#else
		ChMenuItemPtrList		m_items;
		#endif

		ChString					m_strTitle;
		ChMsgHandler			m_pHandler;
		ChMenu*					m_pParent;
		ChRMenuBlock*			m_pBlock;
		chflag32				m_flOptions;
		chflag32				m_flState;
		ChModuleID				m_idModule;
		ChRMenuMgr*				m_pMenuMgr;
};


/*----------------------------------------------------------------------------
	ChHelpMenu class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChHelpMenu : public ChMenu
{
	public:
		ChHelpMenu( ChRMenuMgr* pMgr, ChMsgHandler pHandler, chflag32 flOptions = 0 );
		virtual ~ChHelpMenu() {}
		
		inline ChMenuItem* GetHelpContentsItem()
					{
						return FindItemByID( ID_HELP_INDEX );
					}
};


/*----------------------------------------------------------------------------
	ChEditMenu class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChEditMenu : public ChMenu
{
	public:
		ChEditMenu( ChRMenuMgr* pMgr, ChMsgHandler pHandler, chflag32 flOptions = 0 );
		virtual ~ChEditMenu() {}

		inline ChMenuItem* GetCutItem()
					{
						return FindItemByID( ID_EDIT_CUT );
					}
		inline ChMenuItem* GetCopyItem()
					{
						return FindItemByID( ID_EDIT_COPY );
					}
		inline ChMenuItem* GetPasteItem()
					{
						return FindItemByID( ID_EDIT_PASTE );
					}
};


/*----------------------------------------------------------------------------
	ChFileMenu class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChFileMenu : public ChMenu
{
	public:
		ChFileMenu( ChRMenuMgr* pMgr, ChMsgHandler pHandler, chflag32 flOptions = 0 );
		virtual ~ChFileMenu() {}

		inline ChMenuItem* GetQuitItem()
					{
						return FindItemByID( ID_APP_EXIT );
					};
		virtual chint16 GetTrailingReserved() { return 2; };
};


/*----------------------------------------------------------------------------
	ChViewMenu class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChViewMenu : public ChMenu
{
	public:
		ChViewMenu( ChRMenuMgr* pMgr, ChMsgHandler pHandler, chflag32 flOptions = 0 );
		virtual ~ChViewMenu() {}
};


/*----------------------------------------------------------------------------
	ChWindowMenu class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChWindowMenu : public ChMenu
{
	public:
		ChWindowMenu( ChRMenuMgr* pMgr, ChMsgHandler pHandler, chflag32 flOptions = 0 );
		virtual ~ChWindowMenu() {}
};


/*----------------------------------------------------------------------------
	ChMenuItem class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChMenuItem			//  Should inherit from ChStreamable
{
	friend ChMenuItem* ChMenu::InsertItem( ChString strTitle, 
								const ChMsgHandler pHandler, chint16 sPosition);	
	friend ChMenuItem* ChMenu::InsertItem( ChMenu *pCascadeMenu, chint16 sPosition);
	friend ChMenuItem* ChMenu::InsertSeparator( chint16 sPosition );
	friend class ChRMenuItem;	
	friend class ChRMenuBlock;
	friend class ChMenu;

	public:
		enum ItemType
		{
			CH_MENU_ITEM_TEXT,
			CH_MENU_ITEM_SEPARATOR,
			CH_MENU_ITEM_CASCADE			
		};

	public:
		virtual ~ChMenuItem();

		inline chparam GetData() { return m_data; }
		inline ChMsgHandler GetHandler() { return *m_ppHandler; }
		inline ChString& GetHelpText() { return m_strHelp; }
		inline ChMenu* GetMenu() { return m_pCascade; }
		inline ChMenu* GetParent() { return m_pParent; }
		inline ChString& GetTitle() { return m_strTitle; }
		inline ItemType GetType() { return m_type; }
		inline chuint32 GetAccelerator() const { return m_luAccChar; }
		inline chuint32 GetAccelerator(chflag16& fsMods) const
					{ 
						fsMods = m_fsAccMods;
						return m_luAccChar; 
					}
		inline bool IsChecked()
					{
						return 0 != (m_flState &  CH_STATE_CHECKED);
					}
		inline bool IsEnabled()
					{
						return 0 != (m_flState &  CH_STATE_ENABLED);
					}
		inline ChMenuItem* SetData( chparam data )
					{
						m_data = data;
						return this;
					}

		ChMenuItem* Enable( bool boolEnable = true, bool boolSilent = false );
		ChMenuItem* Check( bool boolCheck = true, bool boolSilent = false );
		ChMenuItem* SetTitle( ChString strTitle );
		ChMenuItem* SetHandler( const ChMsgHandler pHandler = 0 );
		ChMenuItem* SetHelpText( ChString strHelp );
		ChMenuItem* SetAccelerator( const chuint32 luChar, const chflag16 fsMods );
		chint16 GetIndex();

	protected:
		ChMenuItem( ChString strTitle, const ChMsgHandler pHandler, ChMenu *pParent );

		ChMenuItem( ChMenu *pCascadeMenu,  ChMenu *pParent );

		ChMenuItem( ItemType eType, ChMenu *pParent );

		void SetRItem(ChRMenuItem *	pRItem) { m_pRItem = pRItem; };
		inline ChRMenuItem * GetRItem() {return m_pRItem;};
		ChString  GetDisplayTitle( );


	protected:
		ChString			m_strTitle;
		ChString			m_strHelp;
		ChMsgHandler	m_pHandler; 	  
		ChMsgHandler*	m_ppHandler;	
		ChMenu*			m_pCascade;
		ChMenu*			m_pParent;
		chflag32		m_flState;
		ItemType		m_type;
		ChRMenuItem*	m_pRItem;
		chparam			m_data;
		chflag16		m_fsAccMods;
		chuint32		m_luAccChar;
};


class CH_EXPORT_CLASS ChMenuMsg : public ChMsg 
{
	friend class ChRMenuMgr;
	friend class ChRMenuItem;

	public:
		virtual ~ChMenuMsg() {}

		inline ChMenuItem* GetItem() { return m_pItem; }
		inline ChMenu* GetMenu() { return m_pMenu; }

	protected:
		ChMenuMsg( chint32 lMessage = CH_MSG_MENU_SELECT ) :
					ChMsg(lMessage)
				{
				}

		inline void SetItem( ChMenuItem *pItem) { m_pItem = pItem; }
		inline void SetMenu( ChMenu *pMenu) { m_pMenu = pMenu; }

	protected:
		ChMenuItem*	m_pItem;
		ChMenu*		m_pMenu;
};

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR    
#endif

#endif // CHMENU_H

// $Log$
