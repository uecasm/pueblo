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

     Ultra Enterprises:  Gavin Lambert
     
          Rewrote to use new ChFileDialog class.

------------------------------------------------------------------------------

	Interface for the ChLogSaveAsDlg, based on the Windows common
	dialog.  This dialog manages saving log files.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHSAVEAS_H )
#define _CHSAVEAS_H

#if !defined(CH_PUEBLO_PLUGIN)
#include "resource.h"
#else
#include "vwrres.h"
#endif

#include <ChFileDlg.h>

/*----------------------------------------------------------------------------
	ChLogSaveAsDlg class
----------------------------------------------------------------------------*/

class ChLogSaveAsDlg : public ChFileDialog
{
	public:
		ChLogSaveAsDlg( CWnd* pParent = 0 );

		inline bool IsEntireBuffer() { return m_boolEntireBuffer; }
		inline bool IsHTML() { return m_boolHTML; }
		//inline const ChString& GetFileExt() { return m_strFileExtension; }

											// Dialog Data
		//{{AFX_DATA(ChLogSaveAsDlg)
	enum { IDD = IDD_LOG_FILE_SAVE_AS };
	//}}AFX_DATA
											/* ClassWizard generated virtual
												function overrides */
		//{{AFX_VIRTUAL(ChLogSaveAsDlg)
		public:
		virtual int DoModal();
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		virtual void OnTypeChange();
		//}}AFX_VIRTUAL

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChLogSaveAsDlg)
		virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//}}AFX_MSG

		DECLARE_MESSAGE_MAP()

	protected:
		chflag32	m_flSysProps;
		ChString		m_strFilter;
		ChString		m_strDefDir;
		bool		m_boolHTML;
		bool		m_boolEntireBuffer;
};

#endif	// !defined( _CHSAVEAS_H )

// $Log$
