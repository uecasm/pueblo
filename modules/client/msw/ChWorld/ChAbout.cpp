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

	This file consists of implementation of the ChTinTinAbout class.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"
#if !defined(CH_PUEBLO_PLUGIN)
#include "resource.h"
#else
#include "vwrres.h"
#endif

#include "ChAbout.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif


/*----------------------------------------------------------------------------
	ChTinTinAbout property page class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChTinTinAbout, ChPropertyBaseClass )

ChTinTinAbout::ChTinTinAbout() :
				ChPropertyBaseClass( ChTinTinAbout::IDD, 0 
			#if !defined( CH_PUEBLO_PLUGIN )
				,ChWorldDLL.hModule 
			#endif
					)
{
	//{{AFX_DATA_INIT(ChTinTinAbout)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

ChTinTinAbout::~ChTinTinAbout()
{
}


void ChTinTinAbout::DoDataExchange( CDataExchange* pDX )
{
	ChPropertyBaseClass::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChTinTinAbout)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( ChTinTinAbout, ChPropertyBaseClass )
	//{{AFX_MSG_MAP(ChTinTinAbout)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChTinTinAbout message handlers
----------------------------------------------------------------------------*/

// $Log$
