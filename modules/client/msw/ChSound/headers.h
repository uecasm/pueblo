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


/*----------------------------------------------------------------------------
	Switches
----------------------------------------------------------------------------*/

//#if !defined( CH_PUEBLO_PLUGIN )

//#define CH_USE_VOXWARE						// Should be defined for
											// Voxware support
//#endif


/*----------------------------------------------------------------------------
	Includes
----------------------------------------------------------------------------*/

#include <PbSysVersion.h>

#if defined( CH_MSW )

	#include <afxwin.h>						// MFC core and std components
	#include <afxext.h>						// MFC extensions
	#include <afxdisp.h>					// MFC dispatchers               
	#include <afxcmn.h>						// MFC new control classes

	#include <mmsystem.h>					// Multimedia extensions      
	
	#ifndef _MCIDEVICEID_
		#define _MCIDEVICEID_
		typedef UINT    MCIDEVICEID;		// MCI device ID type
	#endif // _MCIDEVICEID_     

	#include <ctype.h>

#endif	// CH_MSW

#if ( _MSC_VER > 900	 )
#pragma warning( disable: 4237 )
#endif

#include <ChTypes.h>
#include <ChModule.h>
#include <ChDispat.h>
#include <ChMsg.h>
#include <ChMsgTyp.h>
#include <ChUtil.h>

#if defined( CH_MSW )

	#if !defined( NO_TEMPLATES )
		#include <afxtempl.h>				// MFC template classes
	#endif

#endif	// CH_MSW


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define VOLUME_MAX_RANGE			0xFFFF

#define PUEBLO_SOUND_CMD			"PUEBLOSOUND"
#define PUEBLO_SOUND_COMPLETE			"complete"

											// Options
const chflag32	soundOptQueue = (1L << 0);

											// Speech options
const chflag32	speechOptStatus = (1L << 0);
const chflag32	speechOptSounds = (1L << 1);
const chflag32	speechOptQuery = (1L << 2);

											// Events
const chflag32	soundEvtComplete = (1L << 0);

/*----------------------------------------------------------------------------
	Sound Preferences registry group
----------------------------------------------------------------------------*/

#define SOUND_PREFS_GROUP				"Sound & Speech"
#define SOUND_PREFS_DISABLED				"Sound disabled?"
#define SOUND_PREFS_DISABLED_DEF				false
#define SOUND_PREFS_MUSIC_VOLUME			"Music volume"
#define SOUND_PREFS_EFFECTS_VOLUME			"Effects volume"
#define SOUND_PREFS_ALERT_VOLUME			"Alert volume"
#define SOUND_PREFS_ALERT_SOUND				"Alert sound"
#define SOUND_PREFS_ALERT_FREQ_COUNT		"Alert frequency"
#define SOUND_PREFS_ALERT_FREQ_COUNT_DEF		60
#define SOUND_PREFS_ALERT_FREQ_UNITS		"Alert frequency units"
#define SOUND_PREFS_ALERT_FREQ_UNITS_SEC		"seconds"
#define SOUND_PREFS_ALERT_FREQ_UNITS_MIN		"minutes"
#define SOUND_PREFS_ALERT_FREQ_UNITS_HOUR		"hours"
#define SOUND_PREFS_ALERT_FREQ_UNITS_DEF		SOUND_PREFS_ALERT_FREQ_UNITS_SEC
#define SOUND_PREFS_REJECT_CALLS			"Reject incoming calls"
#define SOUND_PREFS_REJECT_CALLS_DEF			false
#define SOUND_PREFS_SPEECH_VOLUME			"Speech volume"
#define SOUND_PREFS_SPEECH_VOLUME_DEF			50
#define SOUND_PREFS_MIKE_VOLUME				"Microphone volume"
#define SOUND_PREFS_MIKE_VOLUME_DEF				50


/*----------------------------------------------------------------------------
	Global variables
----------------------------------------------------------------------------*/

#if defined( CH_MSW )

#if !defined( CH_PUEBLO_PLUGIN )
	CH_EXTERN_VAR AFX_EXTENSION_MODULE ChSoundDLL;
#endif

#elif defined( CH_UNIX )

	extern string_resource	sound_resources[];

#endif	// defined( CH_UNIX )


/*----------------------------------------------------------------------------
	Macros
----------------------------------------------------------------------------*/

#if defined( CH_MSW )

#if !defined( CH_PUEBLO_PLUGIN )
	#define LOADSTRING( lKey, strValue ) \
				(ChUtil::Load( (chparam)ChSoundDLL.hModule, lKey, strValue ))

#else
	#define LOADSTRING( lKey, strValue ) \
				(ChUtil::Load( (chparam)AfxGetInstanceHandle(), lKey, strValue ))
#endif

#elif defined( CH_UNIX )

	#define LOADSTRING( lKey, strValue ) \
				(ChUtil::Load( (chparam)sound_resources, lKey, strValue ))

#else

	#error "OS not defined"

#endif


/*----------------------------------------------------------------------------
	Functions
----------------------------------------------------------------------------*/

CH_EXTERN_FUNC( ChString )
GetSysSoundFilesPath();


#endif	// !defined( _HEADERS_H )

// $Log$
// Revision 1.1.1.1  2003/02/03 18:52:56  uecasm
// Import of source tree as at version 2.53 release.
//
