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
	Portions created by Andromedia are Copyright (C) 1998 Andromedia
	Incorporated.  All Rights Reserved.

	Andromedia Incorporated                         415.365.6700
	818 Mission Street - 2nd Floor                  415.365.6701 fax
	San Francisco, CA 94103

    Contributor(s):
	--------------------------------------------------------------------------
	   Ultra Enterprises:   Gavin Lambert

					Wrote and designed this file; it was not originally part of
					the Pueblo client.

------------------------------------------------------------------------------

	This file contains the interface for the PbError library.
	This library is not valid under Borland C++, both because I think it
	uses incompatible APIs and because I couldn't find any way of making
	the Borland compiler produce equivalents of COD files, thereby
	rendering this component fairly pointless anyway.

----------------------------------------------------------------------------*/

// $Header$

#ifndef __BORLANDC__

#include <string>

#ifdef PBERROR_EXPORTS
#define PBERROR_API __declspec(dllexport)
#else
#define PBERROR_API __declspec(dllimport)
#endif

namespace PbError
{
	extern HINSTANCE hInstError;
	extern LPTOP_LEVEL_EXCEPTION_FILTER lpfnPrevExceptionHandler;

	PBERROR_API std::string GetThreadName();
	PBERROR_API void SetThreadName(const char *name);
}

#endif	// ndef(__BORLANDC__)
