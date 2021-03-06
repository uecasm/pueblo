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

	This file consists of the interface for the ChHookManager class, which
	provides functional support for a module to manager hooked messages.

----------------------------------------------------------------------------*/

#if (!defined( _CHHOOK_H ))
#define _CHHOOK_H

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif


#include <ChList.h>
#include <ChMsg.h>



class ChCore;
/*----------------------------------------------------------------------------
	ChHookManager class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChHookManager
{
	public:
		ChHookManager( ChCore* pCore, chparam userData = 0 ) : 
							m_pCore( pCore ), 
							m_userData( userData ) 
							{
							}
		virtual ~ChHookManager() {}

		inline chparam GetUserData() const { return m_userData; }
		inline chparam SetUserData( chparam userData )
						{
							chparam		temp = m_userData;

							m_userData = userData;
							return temp;
						}

		void Install( const ChModuleID& idModule );
		chparam Dispatch( ChMsg& msg, bool& boolProcessed ) const;
		void Promote( const ChModuleID& idModule, bool boolPromote );
		void Uninstall( const ChModuleID& idModule );

	protected:
		ChCore*			m_pCore;
		chparam			m_userData;			// User data field
		ChParamList		m_moduleList;
};

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR    
#endif

#endif	// !defined( _CHHOOK_H )
