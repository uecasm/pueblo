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

					Wrote this file, to be included in all Pueblo/UE source files;
					it was not originally part of the Pueblo client.

------------------------------------------------------------------------------

	This file contains a definition intended for debug builds of Pueblo/UE.
	It causes each heap allocation to be tagged with its origin, which should
	make memory leaks easier to track down.

	It should be included in every Pueblo/UE source file, AFTER all the
	header include directives, and also after any IMPLEMENT_SERIAL calls.

----------------------------------------------------------------------------*/

#ifndef __UE_MEMDEBUG_H
#define __UE_MEMDEBUG_H

#if ( _MFC_VER >= 0x0400 )
	#ifdef _DEBUG
		#ifdef  DEBUG_NEW
			#define new DEBUG_NEW
		#endif
	#endif
#endif

#endif