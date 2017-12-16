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


#if !defined( _HEADERS_H )
#define _HEADERS_H

#include <PbSysVersion.h>

#if defined( CH_MSW )

	#include <afxwin.h>						// MFC core and std components
	#include <afxext.h>						// MFC extensions

	#if _MFC_VER < 0x0710
		// MFC 7.1 includes winsock2.h automatically, and those two don't play nice
		#include <winsock.h>
	#endif

	#if ( _MSC_VER >= 900	 )
		#pragma warning( disable: 4237 )
	#endif

	#include <ChTypes.h>

	#if !defined(CH_PUEBLO_PLUGIN) && !defined( CH_STATIC_LINK )
		CH_EXTERN_VAR AFX_EXTENSION_MODULE	PuebloDLL;
	#endif

#endif

#define CACHE_DIR		"urlcache"

#include <ctype.h>
	

/*----------------------------------------------------------------------------
	Macros
----------------------------------------------------------------------------*/

#if defined( CH_MSW )


	#if !defined( CH_PUEBLO_PLUGIN )

	#define LOADSTRING( lKey, strValue ) \
				(ChUtil::Load( (chparam)PuebloDLL.hModule, lKey, strValue ))

	#else	// !defined( CH_PUEBLO_PLUGIN )

	#define LOADSTRING( lKey, strValue ) \
				(ChUtil::Load( (chparam)AfxGetInstanceHandle(), lKey,\
									strValue ))

	#endif	// !defined( CH_PUEBLO_PLUGIN )

#elif defined( CH_UNIX )

	#define LOADSTRING( lKey, strValue ) \
				(ChUtil::Load( (chparam)world_resources, lKey, strValue ))

#else

	#error "OS not defined"

#endif

#endif	// !defined( _HEADERS_H )
