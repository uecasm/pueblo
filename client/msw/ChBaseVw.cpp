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

	This file contains the implementation of the base Pueblo view class.

----------------------------------------------------------------------------*/

#include "headers.h"
#include "Pueblo.h"

#include "ChBaseVw.h"
#include "MemDebug.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


/*----------------------------------------------------------------------------
	ChBaseView class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE(ChBaseView, CView)

ChBaseView::ChBaseView()
{
}

ChBaseView::~ChBaseView()
{
}


BEGIN_MESSAGE_MAP(ChBaseView, CView)
	//{{AFX_MSG_MAP(ChBaseView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
#if defined( CH_ARCH_16 )	
	ON_WM_CREATE()       
#endif
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChBaseView drawing
----------------------------------------------------------------------------*/

void ChBaseView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

#if defined( CH_ARCH_16 )
int ChBaseView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{   
	lpCreateStruct->style &= ~WS_BORDER;
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;  
	return 0;
	
} 
#endif

/*----------------------------------------------------------------------------
	ChBaseView diagnostics
----------------------------------------------------------------------------*/

#if defined( _DEBUG )

void ChBaseView::AssertValid() const
{
	CView::AssertValid();
}

void ChBaseView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif	// defined( _DEBUG )


/*----------------------------------------------------------------------------
	ChBaseView message handlers
----------------------------------------------------------------------------*/
