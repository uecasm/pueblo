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

	This is the precompiled header for files in this module, and is
	included by all files.

----------------------------------------------------------------------------*/

// $Header$

#if (!defined( _HEADERS_H ))
#define _HEADERS_H

#include <PbSysVersion.h>

#if defined( CH_MSW )

	#include <afxwin.h>         // MFC core and standard components
	#include <afxext.h>         // MFC extensions

	#if ( _MSC_VER > 900	 )

	#pragma warning( disable: 4237 )

	#endif
	
	#include <ChTypes.h>
	#include <ChExcept.h>
	#include <ChUtil.h>

	#include "resource.h"
	#include <ChConst.h>

#define CH_CLIENT							/* Indicates that this is a
												client app */
#endif	// defined( CH_MSW )


/*----------------------------------------------------------------------------
	Global variables
----------------------------------------------------------------------------*/

#if defined( CH_MSW )

	CH_EXTERN_VAR HINSTANCE		hInstApp;

#endif	// defined( CH_MSW )


/*----------------------------------------------------------------------------
	Macros
----------------------------------------------------------------------------*/

#if defined( CH_MSW )

	#define LOADSTRING( lKey, strValue ) \
				(ChUtil::Load( (chparam)hInstApp, lKey, strValue ))

#else

	#error "OS not defined"

#endif

#ifdef _DEBUG
	#define UETRACE(msg) ::MessageBox(0, msg, "UE Trace", MB_OK|MB_ICONINFORMATION)
#else
	#define UETRACE(msg);
#endif

/*----------------------------------------------------------------------------
	Compilation flags
----------------------------------------------------------------------------*/

#define CONNECT_ON_APP_START

#endif	// !defined( _HEADERS_H )

// $Log$
// Revision 1.2  2003/07/04 11:26:42  uecasm
// Update to 2.60 (see help file for details)
//
// Revision 1.1.1.1  2003/02/03 18:52:36  uecasm
// Import of source tree as at version 2.53 release.
//
