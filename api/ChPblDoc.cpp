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

	This file contains the implementation of the base Pueblo document class.
	This document class handles 'pbl' documents, which control basic Pueblo
	functionality.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <ChPblDoc.h>

#ifdef _DEBUG
	#undef THIS_FILE
	static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	ChPuebloDoc class
----------------------------------------------------------------------------*/

ChPuebloDoc::~ChPuebloDoc()
{
}


/*----------------------------------------------------------------------------
	ChPuebloDoc protected methods
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChPuebloDoc, CDocument )

ChPuebloDoc::ChPuebloDoc() 
{
}


BEGIN_MESSAGE_MAP( ChPuebloDoc, CDocument )
	//{{AFX_MSG_MAP(ChPuebloDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChPuebloDoc diagnostics
----------------------------------------------------------------------------*/

#if defined( _DEBUG )

void ChPuebloDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void ChPuebloDoc::Dump( CDumpContext& dc ) const
{
	CDocument::Dump( dc );
}

#endif	// defined( _DEBUG )


/*----------------------------------------------------------------------------
	ChPuebloDoc serialization
----------------------------------------------------------------------------*/

void ChPuebloDoc::Serialize( CArchive& ar )
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


/*----------------------------------------------------------------------------
	ChPuebloDoc commands
----------------------------------------------------------------------------*/

BOOL ChPuebloDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
	{
		return false;
	}

	return true;
}


BOOL ChPuebloDoc::OnOpenDocument( LPCTSTR lpszPathName )
{
	if (!CDocument::OnOpenDocument( lpszPathName ))
	{
		return false;
	}
	return true;
}

// $Log$
