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

// $Header$

#if !defined( _CHBASEVW_H )
#define _CHBASEVW_H


/*----------------------------------------------------------------------------
	ChBaseView class
----------------------------------------------------------------------------*/

class ChBaseView : public CView
{
	protected:
		ChBaseView();						/* Protected constructor used by
												dynamic creation */
		DECLARE_DYNCREATE( ChBaseView )

											/* ClassWizard generated virtual
												function overrides */
		//{{AFX_VIRTUAL(ChBaseView)
		protected:
		virtual void OnDraw(CDC* pDC);      // Overridden to draw this view
		//}}AFX_VIRTUAL

											// Implementation
	protected:
		virtual ~ChBaseView();

		#if defined( _DEBUG )
			virtual void AssertValid() const;
			virtual void Dump(CDumpContext& dc) const;
		#endif

											// Generated message map functions
	protected:
		//{{AFX_MSG(ChBaseView)
			// NOTE - the ClassWizard will add and remove member functions here.
		//}}AFX_MSG  
	#if defined( CH_ARCH_16 )
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	#endif

	DECLARE_MESSAGE_MAP()
};

#endif	// !defined( _CHBASEVW_H )

// $Log$
