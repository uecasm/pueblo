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

#if (!defined( _GRHEADER_H_ ))
#define _GRHEADER_H_

#if defined( CH_MSW )

	#include <afxwin.h>						// MFC core and std components
	#include <afxext.h>						// MFC extensions
	#include <afxdisp.h>					// MFC dispatchers
	#if !defined( NO_TEMPLATES )
	#include <afxtempl.h>					// MFC template classes
	#endif
	#if defined( WIN32 )

		#include <afxcmn.h>					// MFC new control classes

	#endif	// defined( WIN32 )

#if ( _MSC_VER > 900	 )
#pragma warning( disable: 4237 )
#else
	#pragma warning( disable: 4041 )
#endif

#endif	// CH_MSW

#include <ChTypes.h>
#include <ChExcept.h>
#include <ChConst.h>


#if !defined( CH_VRML_PLUGIN )	&& !defined( CH_PUEBLO_PLUGIN )
/*----------------------------------------------------------------------------
	Global variables
----------------------------------------------------------------------------*/

CH_EXTERN_VAR AFX_EXTENSION_MODULE ChGraphicsDLL;

#endif

// $Log$

#endif	// !defined( _GRHEADER_H_ )
