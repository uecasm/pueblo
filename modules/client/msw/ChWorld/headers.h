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

	Source file that includes just the standard includes.  This file may
	be used for precompiled headers.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _HEADERS_H )
#define _HEADERS_H

#include <PbSysVersion.h>

#if defined( CH_MSW )

	#include <afxwin.h>						// MFC core and std components
	#include <afxext.h>						// MFC extensions
	#include <afxdisp.h>					// MFC dispatchers
#endif	// CH_MSW

	#if ( _MSC_VER > 900	 )
	#pragma warning( disable: 4237 )
	#else
											/* Turn off warning that says
												'compiler limit : terminating
												browser output' */
	#pragma warning( disable: 4041 )
	#endif

#include <ChTypes.h>
#include <ChModule.h>
#include <ChDispat.h>
#include <ChMsg.h>
#include <ChMsgTyp.h>
#include <ChUtil.h>
#include <ChWorld.h>

#if defined( CH_MSW )

	#if !defined( NO_TEMPLATES )
	#include <afxtempl.h>					// MFC template classes
	#endif

#endif	// CH_MSW

#include "ChWInfo.h"


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define ATTR_XWORLD					TEXT( "xch_world" )
#define ATTR_XCMD					TEXT( "xch_cmd" )
#define ATTR_XHINT					TEXT( "xch_hint" )
#define ATTR_XMODE					TEXT( "xch_mode" )
#define ATTR_XMODE_HTML				TEXT( "html" )
#define ATTR_XMODE_PURE_HTML		TEXT( "purehtml" )
#define ATTR_XMODE_TEXT				TEXT( "text" )

#define JUMP_PREFIX					"http://"

#define PUEBLO_ENHANCED_PART_1		"pueblo"
#define PUEBLO_ENHANCED_PART_2		"enhanced"
#define PUEBLO_ENHANCED_COMMAND		"PUEBLOCLIENT"

#define PROGMAN_SERVICE				"progman"
#define PROGMAN_TOPIC				"progman"

#define SAVE_LOG_DIR			"Logs"

/*----------------------------------------------------------------------------
	ChWorldList registry group
----------------------------------------------------------------------------*/

#define WORLD_LIST_GROUP		"Personal World list"
#define WORLD_LIST_GROUP_OLD	"World list"
#define WORLD_LIST_ITEM			"World %d"
#define WORLD_LIST_COUNT		"World count"


/*----------------------------------------------------------------------------
	World Preferences registry group
----------------------------------------------------------------------------*/

#define WORLD_PREFS_GROUP				"World module"
#define WORLD_PREFS_ECHO					"Echo input"
#define WORLD_PREFS_ECHO_BOLD				"Echo input/Bold"
#define WORLD_PREFS_ECHO_ITALIC				"Echo input/Italic"
#define WORLD_PREFS_PAUSE_DISCONNECT		"Pause on disconnect"
#define WORLD_PREFS_PAUSE_INLINE			"Pause inline"
#define WORLD_PREFS_NOTIFY_INACTIVE   "Data notify if inactive"
#define WORLD_PREFS_NOTIFY_FLASH			"Data notify flash"
#define WORLD_PREFS_NOTIFY_ALERT			"Data notify alert"
#define WORLD_PREFS_NOTIFY_STR				"Data notify string"

#define WORLD_PREFS_KEYMAP				"Key map"
#define WORLD_PREFS_KEYMAP_DEF				"windows"
#define WORLD_PREFS_CLEAR				"Clear input after send"
#define WORLD_PREFS_CLEAR_DEF				true
#define WORLD_EDIT_LINES				"Number of edit lines"
#define WORLD_EDIT_LINES_DEF				2
#define WORLD_EDIT_LINES_MAX				10
#define WORLD_TINTIN_FILE				"TinTin command file"
#define WORLD_TINTIN_FILE_DEF				""

#define WORLD_PREFS_ALLOWMCCP				"Allow MCCP"
#define WORLD_PREFS_ALLOWMCCP_DEF		true

/*----------------------------------------------------------------------------
	Global variables
----------------------------------------------------------------------------*/

#if defined( CH_MSW )

#if !defined( CH_PUEBLO_PLUGIN )
	CH_EXTERN_VAR AFX_EXTENSION_MODULE ChWorldDLL;
#endif

#elif defined( CH_UNIX )

	extern string_resource	world_resources[];

#endif	// defined( CH_UNIX )


/*----------------------------------------------------------------------------
	Macros
----------------------------------------------------------------------------*/

#if defined( CH_MSW )

#if !defined( CH_PUEBLO_PLUGIN )
	#define LOADSTRING( lKey, strValue ) \
				(ChUtil::Load( (chparam)ChWorldDLL.hModule, lKey, strValue ))
#else
	#define LOADSTRING( lKey, strValue ) \
				(ChUtil::Load( (chparam)AfxGetInstanceHandle(), lKey, strValue ))
#endif

#elif defined( CH_UNIX )

	#define LOADSTRING( lKey, strValue ) \
				(ChUtil::Load( (chparam)world_resources, lKey, strValue ))

#else

	#error "OS not defined"

#endif

#endif	// !defined( _HEADERS_H )

// $Log$
// Revision 1.2  2003/07/04 11:26:43  uecasm
// Update to 2.60 (see help file for details)
//
// Revision 1.1.1.1  2003/02/03 18:53:33  uecasm
// Import of source tree as at version 2.53 release.
//
