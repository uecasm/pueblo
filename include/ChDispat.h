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

	This file contains the interface for the ChDispatcher class.

----------------------------------------------------------------------------*/

// $Header$

#if (!defined( _CHDISPAT_H ))
#define _CHDISPAT_H

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif

#include <ChModule.h>
#include <ChSplay.h>


/*----------------------------------------------------------------------------
	ChMsgHandlerDesc structure
----------------------------------------------------------------------------*/

typedef struct tagChMsgHandlerDesc
{
	chint32			lMessage;
	ChMsgHandler	handler;

} ChMsgHandlerDesc;


/*----------------------------------------------------------------------------
	ChDispatcher class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChDispatcher
{
	public:
		ChDispatcher( ChCore* pCore, const ChModuleID idModule,
						const ChMsgHandler defHandler );
		~ChDispatcher();

		void AddHandler( chint32 lMessage, const ChMsgHandler handler );
		void AddHandler( const ChMsgHandlerDesc* pHandlerDescs, chint16 sCount );

		bool GetHandler( chint32 lMessage, ChMsgHandler& handler );

		chparam Dispatch( ChMsg &msg );

		ChModuleID GetModuleID() const { return m_idModule; }

	private:
		ChCore*			m_pCore;
		ChParamSplay	m_handlerTree;
		ChMsgHandler	m_defHandler;
		ChModuleID		m_idModule;
		ChMainInfo*		m_pMainInfo;

};

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR    
#endif

#endif /* !defined( _CHDISPAT_H ) */

// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
