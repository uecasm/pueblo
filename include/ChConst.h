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

	This file consists of application constants.

----------------------------------------------------------------------------*/

// $Header$

#if (!defined( _CHCONST_H ))
#define _CHCONST_H

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif


/*----------------------------------------------------------------------------
	Color Constants
----------------------------------------------------------------------------*/

/*
#define COLOR_DEF_TEXT					::GetSysColor(COLOR_BTNTEXT)
#define COLOR_DEF_LINK		 			RGB( 0, 0, 0xFF )	   		// blue
#define COLOR_DEF_VIST_LINK 		RGB( 0x80,0,0x80 )	   	// purple
#define COLOR_DEF_ACTIVE_LINK		RGB( 0xFF, 0x00, 0x00)	// red
#define COLOR_DEF_PREFETCH_LINK	RGB( 0x00, 0x80, 0x00 ) // green
#define COLOR_DEF_BACK					::GetSysColor(COLOR_WINDOW)
*/
#define COLOR_DEF_TEXT					RGB(0xC0, 0xC0, 0xC0)		// light gray
#define COLOR_DEF_LINK		 			RGB(0x00, 0xFF, 0xFF)		// cyan
#define COLOR_DEF_VIST_LINK			RGB(0xFF, 0x00, 0xFF)		// purple
#define COLOR_DEF_ACTIVE_LINK		RGB(0x00, 0xFF, 0xFF)		// yellow
#define COLOR_DEF_PREFETCH_LINK	RGB(0x00, 0xFF, 0x00)		// green
#define COLOR_DEF_BACK					RGB(0x00, 0x00, 0x00)		// black


/*----------------------------------------------------------------------------
	Registry Constants
----------------------------------------------------------------------------*/ 

#define CH_GENERAL_GROUP			"General"
#define CH_PRODUCT_ID				"Product ID"
#define CH_REGISTERED				"Register"
#define CH_UNREGISTERED_USER		0
#define CH_REGISTERED_USER			1
#define CH_REGISTRATION_NOTIFIED	2

#define CH_DEBUG_GROUP				"Debug"
#define CH_DEBUG_USE_LOCAL				"Use local modules"

#define CH_APPS_GROUP				"Applications"
#define	CH_APP_WEBBROWSER			"Web Browser"
#define CH_APP_WEBBROWSER_DEF		""
#define	CH_APP_WEBTRACKER			"WebTracker"
#define CH_APP_WEBTRACKER_DEF			0
#define	CH_APP_DEFAULTBROWSER			"ShellDefault"

#define CH_LOGIN_GROUP				"Login"
#define CH_LOGIN_LAST_NAME				"Last_Login_Name"
#define CH_LOGIN_NAMES					"Names_List"

#define CH_MISC_GROUP				"Miscellaneous"
#define CH_MISC_LICENSE_ACCEPTED		"License Accepted"
#define CH_MISC_TRACE_OPTIONS		"Trace Options"

#define CH_FONT_GROUP				"Fonts"
#define CH_FONT_PROPORTIONAL			"Proportional Font"
#define CH_FONT_PROPORTIONAL_DEF			"Arial"
#define CH_FONT_PROPORTIONAL_SIZE		"Proportional Point Size"
#define CH_FONT_PROPORTIONAL_SIZE_DEF		10
#define CH_FONT_FIXED					"Fixed Font"
#define CH_FONT_FIXED_DEF					"Terminal"
#define CH_FONT_FIXED_SIZE				"Fixed Point Size"
#define CH_FONT_FIXED_SIZE_DEF				10

#define CH_COLOR_GROUP					"Colors"
#define CH_COLOR_DEFAULT				0x80000000
#define CH_COLOR_BACK					"HTML background"
#define CH_COLOR_TEXT					"HTML text"
#define CH_COLOR_LINK					"HTML links"
#define CH_COLOR_FLINK					"HTML followed links"
#define CH_COLOR_PLINK					"HTML prefetched links"

#define CH_CACHE_GROUP				"URL Cache"
#define CH_CACHE_DIR					"CacheDir"
#define CH_CACHE_SIZE					"CacheSize"
#define CH_CACHE_SIZE_DEF				5000
#define CH_CACHE_OPTION					"CacheOptions"
#define CH_CACHE_VERIFY_PER_SESSION		0x01
#define CH_CACHE_VERIFY_EVERYTIME		0x02
#define CH_CACHE_VERIFY_NEVER			0x04
#define CH_CACHE_OPTION_DEF				CH_CACHE_VERIFY_PER_SESSION


#define CH_NETWORK_GROUP			"Network"
#define CH_MAX_CONNECTIONS			"Maximum Connections"
#define CH_MAX_CONNECTIONS_DEF		"5"

#define CH_PROXIES_GROUP			"Proxies"
#define CH_PROXIES					"Use proxies?"
#define CH_PROXIES_DEF					false
#define CH_HTTP_PROXY				"HTTP Proxy"
#define CH_HTTP_PROXY_DEF				""
#define CH_HTTP_PROXY_PORT			"HTTP Proxy Port"
#define CH_HTTP_PROXY_PORT_DEF			0
#define CH_FTP_PROXY				"FTP Proxy"
#define CH_FTP_PROXY_DEF				""
#define CH_FTP_PROXY_PORT			"FTP Proxy Port"
#define CH_FTP_PROXY_PORT_DEF			0
#define CH_SOCKS_PROXY				"Socks Proxy"
#define CH_SOCKS_PROXY_DEF				""
#define CH_SOCKS_PROXY_PORT			"Socks Proxy Port"
#define CH_SOCKS_PROXY_PORT_DEF			1080

#define CH_LAYOUT_GROUP				"Window layout"
#define CH_LAYOUT_SWAPPED				"Swapped panes"
#define CH_LAYOUT_VERTICAL				"Vertical panes"
#define CH_LAYOUT_PANE_SIZE				"Pane size %d"
#define CH_LAYOUT_PANE_SIZE_DEF				200

#define CH_DEBUG_GROUP				"Debug"
#define CH_DEBUG_USE_LOCAL				"Use local modules"		 



/*----------------------------------------------------------------------------
	Internal message  Constants
----------------------------------------------------------------------------*/

											// Html font change message

#define WM_HTML_FONT_CHANGE			(WM_USER + 400)

											// Html default color change message

#define WM_HTML_COLOR_CHANGE		(WM_USER + 401)

											// HTTP notification message 
											// WPARAM = 0
											// LPARAM = ChHTTPNotification*

#define WM_CHACO_HTTP_MSG			(WM_USER + 402)
	
											// Vrml preference change message

#define WM_VRML_PREF_CHANGE			(WM_USER + 403)

											// VRML Progress messages
											// WPARAM = Message ID
											// LPARAM = 0

#define WM_VRML_PROGRESS			(WM_USER + 404)

											// VRML Progress messages
											// WPARAM = Message ID
											// LPARAM = Pointer to ChParseInfo

#define WM_VRML_PARSE_DONE			(WM_USER + 405)

											// VRML Progress messages
											// WPARAM = Message ID
											// LPARAM = Ptr to message string
											/*** Note if you change the ID of
													this constant then update
													QvReadError.cpp file */
#define WM_VRML_PARSE_ERROR			(WM_USER + 406)

											/* The native WM_ACTIVATE is mapped
												to WM_VRML_ACTIVATE to prevent
												any other windows from
												responding to the message */

#define WM_VRML_ACTIVATE			(WM_USER + 407)

											/* Sent to the view on successful
												decoding of texture and
												converting into native format */

											// LPARAM = ChMazeTextureHTTPReq*

#define WM_VRML_LOAD_TEXTURE		(WM_USER + 409)

											// For event-based Vrml 2.0 API

#define WM_VRML_EVENT				(WM_USER + 410)

											/* Posted to a window to tell it
												to grab the focus */

#define WM_CHACO_GRABFOCUS			(WM_USER + 420)

											// HTTP notification message 
											// WPARAM = 0
											// LPARAM = *

#define WM_MODULE_HTTP_REQ_MSG		(WM_USER + 430)

											// Async dispatch message 
											// WPARAM = idModule
											// LPARAM = ChMsg*

#define WM_CHACO_ASYNC_DISPATCH		(WM_USER + 431)

											// Script message 
											// WPARAM = 0
											// LPARAM = pstrScript file

#define WM_EXECUTE_PUEBLO_SCRIPT	(WM_USER + 432)

											// Text window recompute message 
											// WPARAM = 0
											// LPARAM = 0

#define WM_TXTWND_RECOMPUTE_VIEW	(WM_USER + 433)

											// VRML error messages
											// WPARAM = error code
											// LPARAM = Ptr to string*
#define WM_VRML_MAZE_ERROR			(WM_USER + 434)


#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	#undef AFXAPP_DATA
	#define AFXAPP_DATA NEAR    
#endif

// $Log$
// Revision 1.1.1.1  2003/02/03 18:55:35  uecasm
// Import of source tree as at version 2.53 release.
//

#endif	// !defined( _CHCONST_H )
