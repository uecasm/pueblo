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

	WebTracker API's

----------------------------------------------------------------------------*/

#if !defined( _CHWEBTRACKER_H_ )
#define _CHWEBTRACKER_H_


#if defined( CH_MSW ) && defined( CH_ARCH_32 )
											/* Disable warnings about non-
												exported classes for MSW */
	#pragma warning( disable: 4273 )

#endif	// defined( CH_MSW )


#define		WEBTRACKER 			"_WebTracker" 


class CH_EXPORT_CLASS ChWebTracker
{
	public :
		static CSingleDocTemplate* GetDocumentTemplate();
		static bool LoadWebTracker( const ChString& strURL, const char* pstrFrameName = 0,
													CWnd* pOwner  = 0  );
		static void CloseAllFrames();
};

#endif // _CHWEBTRACKER_H_
