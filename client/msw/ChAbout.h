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
     
          Modified to use ChPropertySheet (instead of CPropertySheet),
          which includes modified behaviour to allow using a custom
          font in the property sheet.
------------------------------------------------------------------------------

	This file consists of interfaces of the ChAbout class, ChSplashWnd class,
	and ChBigIcon class.

----------------------------------------------------------------------------*/

// $Header$

#if (!defined( _CHABOUT_H ))
#define _CHABOUT_H

#include "ChPagMgr.h"

#include <ChDibImage.h>
#include <ChHtmWnd.h>
#include <ChPage.h>
#include <ChProp.h>


/*----------------------------------------------------------------------------
				ChLogoBmp class
----------------------------------------------------------------------------*/

class ChLogoBitmap : public CButton
{
	public:
		enum tagLogoAlign{ top = 0x1, bottom = 0x2, vcenter = 0x4,
							left = 0x8, right = 0x10, hcenter = 0x20 };
	public:
		void SizeToContent( chflag16 fAlignment, WORD resId );

	protected:
		ChDib		m_logoBmp;
		chint32		m_lBmpHeight;
		chint32		m_lBmpWidth;

		virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );

		//{{AFX_MSG(ChLogoBitmap)
		afx_msg BOOL OnEraseBkgnd(CDC* pDC);
		//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
};


/*----------------------------------------------------------------------------
	ChSplashWnd class
----------------------------------------------------------------------------*/

class ChSplashWnd : public CDialog
{
	public:
		bool Create( CWnd* pParent );
											// Dialog Data
		//{{AFX_DATA(ChSplashWnd)
		enum { IDD = IDD_SPLASH };
		CStatic	m_staticCopyright2;
		CStatic	m_staticCopyright1;
		CStatic	m_staticVersionString;
		//}}AFX_DATA

	protected:
	    									// DDX/DDV support

		virtual void DoDataExchange( CDataExchange* pDX );

	protected:
		ChLogoBitmap	logoBmp;

		// Generated message map functions
		//{{AFX_MSG(ChSplashWnd)
		virtual BOOL OnInitDialog();
		afx_msg void OnDestroy();
		//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};


/*----------------------------------------------------------------------------
	ChPuebloAbout property page class
----------------------------------------------------------------------------*/

class ChPuebloAbout : public ChPropertyPage
{
	DECLARE_DYNCREATE( ChPuebloAbout )

	public:
		ChPuebloAbout( );
		~ChPuebloAbout();
											// Overrides
		virtual BOOL OnSetActive();

	protected:
											// Dialog data
		//{{AFX_DATA(ChPuebloAbout)
		enum { IDD = IDD_ABOUT_BOX };
		CStatic	m_staticVersion;
		CStatic	m_staticProductName;
		CStatic	m_staticLegend;
		CStatic	m_staticCopyright1;
		CStatic	m_staticCopyright2;
		CStatic	m_staticCopyright3;
		CStatic	m_staticClause;
		//}}AFX_DATA
											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChPuebloAbout)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
		ChLogoBitmap	m_logoBmp;

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChPuebloAbout)
		virtual BOOL OnInitDialog();
		//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};


/*----------------------------------------------------------------------------
	ChDisclaimerAbout property page class
----------------------------------------------------------------------------*/

class ChDisclaimerAbout : public ChPropertyPage
{
	DECLARE_DYNCREATE( ChDisclaimerAbout )

	public:
		ChDisclaimerAbout();
		~ChDisclaimerAbout();

		void CreateTextWindow();
											// Overrides
		virtual BOOL OnSetActive();

	protected:
											// Dialog data
		//{{AFX_DATA(ChDisclaimerAbout)
		enum { IDD = IDD_ABOUT_DISCLAIMER };
			// NOTE - ClassWizard will add data members here.
			//    DO NOT EDIT what you see in these blocks of generated code !
		//}}AFX_DATA


											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChDisclaimerAbout)
		protected:
		virtual void DoDataExchange( CDataExchange* pDX );    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChDisclaimerAbout)
		virtual BOOL OnInitDialog();
		//}}AFX_MSG

	protected:
		ChLogoBitmap	m_logoBmp;
		ChHtmlWnd		m_htmlWnd;

	DECLARE_MESSAGE_MAP()
};


/*----------------------------------------------------------------------------
	ChTeamAbout property page class
----------------------------------------------------------------------------*/

class ChTeamAbout : public ChPropertyPage
{
	DECLARE_DYNCREATE( ChTeamAbout )

	public:
		ChTeamAbout();
		~ChTeamAbout();

											// Overrides
		virtual BOOL OnSetActive();

	protected:
		void RandomizeNames();
		int GetRandomInt( int iMax );
		void SetNames();

	protected:
		enum tagConst { namesCount = 5 };

		static chint32		iTeamNames[namesCount];

	protected:
											// Dialog Data
		//{{AFX_DATA(ChTeamAbout)
		enum { IDD = IDD_ABOUT_TEAM };
		CStatic	m_staticName5;
		CStatic	m_staticName4;
		CStatic	m_staticName3;
		CStatic	m_staticName2;
		CStatic	m_staticName1;
		CStatic	m_staticTeamTitle;
		//}}AFX_DATA


											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChTeamAbout)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChTeamAbout)
		virtual BOOL OnInitDialog();
		//}}AFX_MSG

	protected:
		ChLogoBitmap	m_logoBmp;

	DECLARE_MESSAGE_MAP()
};


/*----------------------------------------------------------------------------
	ChUEAbout property page class
----------------------------------------------------------------------------*/

class ChUEAbout : public ChPropertyPage
{
	DECLARE_DYNCREATE( ChUEAbout )

	public:
		ChUEAbout( );
		~ChUEAbout();
											// Overrides
		virtual BOOL OnSetActive();

	protected:
											// Dialog data
		//{{AFX_DATA(ChUEAbout)
		enum { IDD = IDD_ABOUT_UE };
		CStatic	m_staticCompanyName;
		CStatic m_staticWebsite;
		//}}AFX_DATA
											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChUEAbout)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
		ChLogoBitmap	m_logoBmp;

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChUEAbout)
		virtual BOOL OnInitDialog();
		//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};


class ChCore;
/*----------------------------------------------------------------------------
	ChAbout property sheet class
----------------------------------------------------------------------------*/

class ChAbout : public ChPropertySheet
{
	DECLARE_DYNAMIC( ChAbout )

	public:
		ChAbout( ChCore* pCore, chuint16 suIDCaption, CWnd *pParentWnd = 0,
					chuint16 suSelectPage = 0 );
		ChAbout( ChCore* pCore, char *pstrCaption, CWnd *pParentWnd = 0,
					chuint16 suSelectPage = 0 );

		void AddModulePages();
											/* ClassWizard generated virtual
												function overrides */
		//{{AFX_VIRTUAL(ChAbout)
		//}}AFX_VIRTUAL

	public:
		virtual ~ChAbout();

	protected:
		ChPageManager		m_pageMgr;

	protected:
		//{{AFX_MSG(ChAbout)
		afx_msg void OnOK();
		afx_msg void OnCancel();
		afx_msg void OnHelp();
		afx_msg void OnClose();
		virtual BOOL OnInitDialog();
		//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#endif	// !defined( _CHABOUT_H )

// $Log$
