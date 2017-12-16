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

					Created this class.

------------------------------------------------------------------------------

	This file contains the implementation for the ChUtil32 library version provider.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"
#include <ChTypes.h>

// keep gif_lib happy
#undef DrawText

#include "zlib.h"
#include "libmng.h"
#include "jpeglib.h"
#include "lcms.h"

static char buffer[32];

CH_GLOBAL_LIBRARY(const char *)
GetZLibVersion()
{
	lstrcpy(buffer, "zlib version " ZLIB_VERSION);
	return buffer;
}

CH_GLOBAL_LIBRARY(const char *)
GetLibMngVersion()
{
	lstrcpy(buffer, "libmng version " MNG_VERSION_TEXT);
	return buffer;
}

CH_GLOBAL_LIBRARY(const char *)
GetJpegLibVersion()
{
	wsprintf(buffer, "jpeglib version %d.%d", JPEG_LIB_VERSION / 10, JPEG_LIB_VERSION % 10);
	return buffer;
}

CH_GLOBAL_LIBRARY(const char *)
GetLCMSVersion()
{
	wsprintf(buffer, "lcms version %d.%02d", LCMS_VERSION / 100, LCMS_VERSION % 100);
	return buffer;
}

// $Log$
