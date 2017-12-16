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

	This file contains the interface for the base Pueblo document class.
	This document class handles 'pbl' documents, which control basic Pueblo
	functionality.

----------------------------------------------------------------------------*/

#if !defined( _CHPBLDOC_H )
#define _CHPBLDOC_H

#if defined( CH_MSW )
											/* Disable warnings about non-
												exported classes for MSW */
	#pragma warning( disable: 4275 )
	#pragma warning( disable: 4251 )

#endif	// defined( CH_MSW )


/*----------------------------------------------------------------------------
	ChPuebloDoc class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChPuebloDoc : public CDocument
{
	public:
		virtual ~ChPuebloDoc();


	protected:
		ChPuebloDoc();						/* Protected constructor used by
												dynamic creation */
		DECLARE_DYNCREATE( ChPuebloDoc )

											/* ClassWizard generated virtual
												function overrides */
		//{{AFX_VIRTUAL(ChPuebloDoc)
		public:
		virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
		protected:
		virtual BOOL OnNewDocument();
		//}}AFX_VIRTUAL

	public:
		virtual void Serialize( CArchive& ar );

		#if defined( _DEBUG )

			virtual void AssertValid() const;
			virtual void Dump(CDumpContext& dc) const;

		#endif	// defined( _DEBUG )

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChPuebloDoc)
			// NOTE - the ClassWizard will add and remove member functions here.
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()

	protected:
};

#endif	// !defined( _CHPBLDOC_H )
