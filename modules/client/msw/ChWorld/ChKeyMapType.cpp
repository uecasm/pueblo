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

	Contains the implementation of the ChKeyMapType class.

----------------------------------------------------------------------------*/

// $Header$


#include "headers.h"

#include <ChArch.h>
#include "ChKeyMapType.h"
#include "MemDebug.h"

#define _countof( arr )		(sizeof( arr ) / sizeof( arr[0] ))


/*----------------------------------------------------------------------------
	ChKeyMapType class
----------------------------------------------------------------------------*/

											/* 'keymapEndList' must be the
												last in the following list */
KeyMapName		ChKeyMapType::m_mapNames[] =
							{	{ keymapWindows, "windows" },
								{ keymapEmacs, "emacs" },
								{ keymapEndList, "" } };

KeyMapEntry		ChKeyMapType::m_windowsKeyMap[] =
							{	{ VK_RETURN, 0, ACTION_SEND, actSend },
								{ VK_TAB, 0, ACTION_TAB_COMPLETION, actTabCompletion },
								{ VK_PRIOR, 0, ACTION_LOG_PAGE_UP, actLogPageUp },
								{ VK_PRIOR, ACTION_MOD_CONTROL, ACTION_LOG_HOME, actLogHome },
								{ VK_NEXT, 0, ACTION_LOG_PAGE_DOWN, actLogPageDown },
								{ VK_NEXT, ACTION_MOD_CONTROL, ACTION_LOG_END, actLogEnd },
								{ 'T', ACTION_MOD_CONTROL, ACTION_TRANSPOSE, actTranspose },
								{ VK_ESCAPE, 0, ACTION_DELETE_TEXT, actDeleteText },
								{ VK_UP, 0, ACTION_CURSOR_UP, actCursorUp },
								{ VK_DOWN, 0, ACTION_CURSOR_DOWN, actCursorDown },
								{ VK_UP, ACTION_MOD_ALT, ACTION_HISTORY_PREV, actHistoryPrev },
								{ VK_DOWN, ACTION_MOD_ALT, ACTION_HISTORY_NEXT, actHistoryNext },
								{ VK_UP, ACTION_MOD_CONTROL, ACTION_CURSOR_UP_WITHINCMD, actCursorUpWithinCmd },
								{ VK_DOWN, ACTION_MOD_CONTROL, ACTION_CURSOR_DOWN_WITHINCMD, actCursorDownWithinCmd },
							};

KeyMapEntry		ChKeyMapType::m_emacsKeyMap[] =
							{	{ VK_RETURN, 0, ACTION_SEND, actSend },
								{ VK_TAB, 0, ACTION_TAB_COMPLETION, actTabCompletion },
								{ VK_PRIOR, 0, ACTION_LOG_PAGE_UP, actLogPageUp },
								{ VK_PRIOR, ACTION_MOD_CONTROL, ACTION_LOG_HOME, actLogHome },
								{ VK_NEXT, 0, ACTION_LOG_PAGE_DOWN, actLogPageDown },
								{ VK_NEXT, ACTION_MOD_CONTROL, ACTION_LOG_END, actLogEnd },
								{ 'T', ACTION_MOD_CONTROL, ACTION_TRANSPOSE, actTranspose },
								{ 'P', ACTION_MOD_CONTROL, ACTION_CURSOR_UP, actCursorUp },
								{ VK_UP, 0, ACTION_CURSOR_UP_2, actCursorUp },
								{ 'N', ACTION_MOD_CONTROL, ACTION_CURSOR_DOWN, actCursorDown },
								{ VK_DOWN, 0, ACTION_CURSOR_DOWN_2, actCursorDown },
								{ 'B', ACTION_MOD_CONTROL, ACTION_CURSOR_LEFT, actCursorLeft },
								{ 'F', ACTION_MOD_CONTROL, ACTION_CURSOR_RIGHT, actCursorRight },
								{ 'A', ACTION_MOD_CONTROL, ACTION_CURSOR_START_LINE, actCursorStartLine },
								{ 'E', ACTION_MOD_CONTROL, ACTION_CURSOR_END_LINE, actCursorEndLine },
								{ 'K', ACTION_MOD_CONTROL, ACTION_DELETE_TO_END_BUFF, actDeleteToEndOfBuffer },
								{ 'D', ACTION_MOD_CONTROL, ACTION_DELETE_NEXT_CHAR, actDeleteNextChar },
								{ 'U', ACTION_MOD_CONTROL, ACTION_DELETE_TEXT, actDeleteText } };

ChKeyMapType::ChKeyMapType( const ChString& strType )
{
	Set( strType );
}


ChKeyMapType::ChKeyMapType( const ChKeyMapType& type )
{
	m_map = type.m_map;
}


ChKeyMapType::ChKeyMapType( KeymapType type )
{
	m_map = type;
}


ChString ChKeyMapType::GetName() const
{
	KeyMapName*		pTable = ChKeyMapType::m_mapNames;
	ChString			strType;

 	while ((keymapEndList != pTable->type) && (0 == strType.GetLength()))
 	{
		if (pTable->type == m_map)
		{
			strType = pTable->pstrName;
		}
		else
		{
			pTable++;
		}
	}

	return strType;
}


void ChKeyMapType::Set( const ChString& strType )
{
	KeyMapName*	pTable = m_mapNames;
	
	m_map = keymapUndefined;

	while ((keymapEndList != pTable->type) && (keymapUndefined == m_map))
	{
		if (0 == strType.CompareNoCase( pTable->pstrName ))
		{
			m_map = pTable->type;
		}
		else
		{
			pTable++;
		}
	}
											/* If nothing else was found, use
												'Windows' */
	if (keymapUndefined == m_map)
	{
		m_map = keymapWindows;
	}
}


void ChKeyMapType::CreateMap( ChKeyMap& keyMap )
{
	KeyMapEntry*	pKeyMapEntry;
	int				iCount;

	keyMap.Empty();

	switch( m_map )
	{
		case keymapEmacs:
		{
			pKeyMapEntry = m_emacsKeyMap;
			iCount = _countof( m_emacsKeyMap );
			break;
		}

		default:
		{
			pKeyMapEntry = m_windowsKeyMap;
			iCount = _countof( m_windowsKeyMap );
			break;
		}
	}

	for (int iLoop = 0; iLoop < iCount; iLoop++)
	{
		keyMap.AddItem( pKeyMapEntry->luKey, pKeyMapEntry->flModifiers,
						pKeyMapEntry->pstrName, pKeyMapEntry->userData );
		pKeyMapEntry++;
	}
}

// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
// Revision 1.2  2003/07/04 11:26:42  uecasm
// Update to 2.60 (see help file for details)
//
// Revision 1.1.1.1  2003/02/03 18:53:09  uecasm
// Import of source tree as at version 2.53 release.
//
