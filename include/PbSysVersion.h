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

	This file contains a few basic API declarations intended to be included
	by every single sub-project within Pueblo/UE.

----------------------------------------------------------------------------*/

// $Header$

#ifndef __PBSYSVERSION_H
#define __PBSYSVERSION_H

#ifdef CH_MSW
	// We're trying to target Windows 95 and above, as well as NT4 and above.
	//
	// Note that if one of these has already been specified elsewhere we'll
	// use that version instead; this is because some source files explicitly
	// require more advanced features.  In such cases we just have to be
	// careful :)
# ifndef WINVER
#  define WINVER				0x0400
# endif
# ifndef _WIN32_WINNT
#  define _WIN32_WINNT	0x0400
# endif
# ifndef _WIN32_IE
#  define _WIN32_IE			0x0400
# endif

#endif	// CH_MSW

#endif
