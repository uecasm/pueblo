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

	Interface for the ChSoundOpenFileDlg, based on the Windows common
	dialog.  This dialog manages opening sound files.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHSOPEN_H )
#define _CHSOPEN_H

#if !defined(CH_PUEBLO_PLUGIN )
#include "resource.h"
#else
#include "vwrres.h"
#endif

#include "ChFileDlg.h"
#include "ChMPlay.h"


/*----------------------------------------------------------------------------
	ChSoundOpenFileDlg class
----------------------------------------------------------------------------*/

class ChSoundOpenFileDlg : public ChFileDialog
{
	public:
		ChSoundOpenFileDlg( ChSoundMainInfo* pMainInfo, chuint16 suVolume,
							const ChString& strAlertSoundPath,
							const ChString& strAlertSoundTitle,
							CWnd* pParent = 0 );

		inline void Test() { OnTest(); }
		inline void NoSound() { OnNoSound(); }
											// Dialog Data
		//{{AFX_DATA(ChSoundOpenFileDlg)
		enum { IDD = IDD_SOUND_FILE_OPEN };
		//}}AFX_DATA
											/* ClassWizard generated virtual
												function overrides */
		//{{AFX_VIRTUAL(ChSoundOpenFileDlg)
		public:
			virtual int DoModal();
		protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
		inline ChSoundMainInfo* GetMainInfo() { return m_pMainInfo; }

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChSoundOpenFileDlg)
		afx_msg void OnNoSound();
		afx_msg void OnTest();
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()

	protected:
		ChSoundMainInfo*	m_pMainInfo;
		chflag32			m_flSysProps;
		chuint16			m_suVolume;
		ChString				m_strInitialTitle;
		ChString				m_strInitialPath;
		bool				m_boolNoSound;
};


#endif	// !defined( _CHSOPEN_H )

// $Log$
