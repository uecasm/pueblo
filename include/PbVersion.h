/*----------------------------------------------------------------------------
  _   _ _  _                 ____        _                     _
 | | | | || |               |  __|      | |                   (_)
 | | | | || |_ _ __  __ _   | |_  _ __ _| |_ ___ _ ___ __ _ __ _ ___  ___ ___
 | | | | |_  _| '__|/ _` |  |  _|| '_ \_   _| _ \ '__|'_ \ '__| | __|/ _ \ __|
 | '-' | || | | |  | (_| |  | |__| | | || ||  __/ | | |_) ||  | |__ |  __/__ |
  \___/|_||_| |_|   \__,_|  |____|_| |_||_| \___|_| | .__/_|  |_|___|\___|___|
                                                    | |     
                                                    |_|

    The contents of this file are subject to the Andromedia Public
	License Version 1.0 (the "License"); you may not use this file
	except in compliance with the License. You may obtain a copy of
	the License at http://pueblo.sf.net/APL/

    Software distributed under the License is distributed on an
	"AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
	implied. See the License for the specific language governing
	rights and limitations under the License.

    The Original Code is Pueblo/UE client code, first released April 1, 2002.

    The Initial Developer of the Original Code is Ultra Enterprises.
	
    Contributor(s):
	--------------------------------------------------------------------------
	   Ultra Enterprises:   Gavin Lambert

					Wrote this file; it was not originally part of the Pueblo client.

------------------------------------------------------------------------------

	This file consists of definitions for Pueblo/UE's resource and code files
	to use, so that there is one central location to modify when the program
	version changes.  It was really getting on my nerves, having to modify
	the version number in so many places!

----------------------------------------------------------------------------*/

// $Header$

// Define this if this is a prerelease version; it will result in slightly
// different version displays.
//#define UE_PRERELEASE

// First of all, the version info used by the program code:
#define VERS_CLIENT_MAJOR			2
#define VERS_CLIENT_MINOR			60
#define VERS_CLIENT_COMMENT		"UE (b)"

// And now that for the resource files:
#define VERS_CLIENT_BINARY		2,6,0,4
#define VERS_CLIENT_TEXT			"2.60b\0"
#define VERS_CLIENT_COMPANY		"Ultra Enterprises\0"
#define VERS_CLIENT_COPYRIGHT	"Recompiled 2002-03 by Ultra Enterprises\r\nCopyright © 1996-1998 Andromedia Incorporated\0"
#define VERS_CLIENT_PRODUCT		"Pueblo/UE Application\0"

// The following constant enables the version check on startup; it should
// *NOT* be switched on for any unofficial releases (as that will confuse
// the issue, making it check against unrecognised version numbers).
// Just leave it alone, please! :)
//#define UE_VERSION_CHECK

// This is just a sanity check; if we're doing a prerelease build then we
// don't want any version checking, as it will always flag a "new version"
// warning until the actual release has been entered into the system.
#ifdef UE_PRERELEASE
# undef UE_VERSION_CHECK
#endif
