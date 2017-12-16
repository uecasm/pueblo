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

	This file contains the interface for the ChPrefsFontPage class,
	which allows the user to select font preferences.

----------------------------------------------------------------------------*/

#if !defined( _CHPRFONT_H )
#define _CHPRFONT_H

#include <ChReg.h>
#include <ChPage.h>


/*----------------------------------------------------------------------------
	ChPrefsFontPage class
----------------------------------------------------------------------------*/

class ChPrefsFontPage : public ChPropertyPage
{
	DECLARE_DYNCREATE( ChPrefsFontPage )

	public:
		static const chuint32	ChPrefsFontPage::luBackColors[];

	public:
		ChPrefsFontPage();
		~ChPrefsFontPage();
											// Overrides
		virtual BOOL OnSetActive();
		virtual void OnCommit();
											// Dialog Data
		//{{AFX_DATA(ChPrefsFontPage)
		enum { IDD = IDD_PREF_FONT };
		CComboBox	m_comboBackColor;
		CStatic	m_staticProportionalName;
		CStatic	m_staticFixedName;
		int		m_iBackColor;
		//}}AFX_DATA
											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChPrefsFontPage)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
		void InitColorList( CComboBox& comboColor );
		void DrawColorItem( LPDRAWITEMSTRUCT lpDrawItemStruct, CDC* pDC,
								const CRect& rtItem );
		void DrawColorSelect( LPDRAWITEMSTRUCT lpDrawItemStruct,
								CDC* pDC, const CRect& rtItem,
								bool boolSelected );

											// Generated message map functions
		//{{AFX_MSG(ChPrefsFontPage)
		afx_msg void OnChooseFixedFont();
		afx_msg void OnChooseProportionalFont();
		afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
		afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
		//}}AFX_MSG

	protected:
		bool		m_boolInitialized;
		ChString		m_strProportionalFont;
		chint32		m_lProportionalPointSize;
		ChString		m_strFixedFont;
		chint32		m_lFixedPointSize;
		ChRegistry	m_regFont;

	DECLARE_MESSAGE_MAP()
};


/*----------------------------------------------------------------------------
	ChPrefsColorPage class
----------------------------------------------------------------------------*/

class ChPrefsColorPage : public ChPropertyPage
{
	DECLARE_DYNCREATE( ChPrefsColorPage )

	public:
		ChPrefsColorPage();
		~ChPrefsColorPage();
											// Overrides
		virtual BOOL OnSetActive();
		virtual void OnCommit();
											// Dialog Data
		//{{AFX_DATA(ChPrefsColorPage)
		enum { IDD = IDD_PREF_COLORS };
		CButton	m_btnSampleBack;
		CButton	m_btnSampleText;
		CButton	m_btnSampleLink;
		CButton	m_btnSamplePLink;
		CButton	m_btnSampleFLink;
		CComboBox	m_comboBackColor;
		CComboBox	m_comboTextColor;
		CComboBox	m_comboLinkColor;
		CComboBox	m_comboFLinkColor;
		CComboBox	m_comboPLinkColor;
		int			m_iBackColor;
		int			m_iTextColor;
		int			m_iLinkColor;
		int			m_iFLinkColor;
		int			m_iPLinkColor;
		//}}AFX_DATA
											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChPrefsColorPage)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
		inline bool IsDefColor( chuint32 luColor )
						{
							return ( (CH_COLOR_DEFAULT & luColor) != 0 );
						}
		void InitColorList( CComboBox& comboColor,
								chuint16 suExtraColorText = 0 );
		chint16 GetColorIndex( chuint32 luColor, bool boolDefaultUsed = true );
		chuint32 GetColorValue( chint16 sIndex, bool boolDefaultUsed = true );
		void DrawColorItem( LPDRAWITEMSTRUCT lpDrawItemStruct, CDC* pDC,
								const CRect& rtItem );
		void DrawColorSelect( LPDRAWITEMSTRUCT lpDrawItemStruct,
								CDC* pDC, const CRect& rtItem,
								bool boolSelected );
		void UpdateColorSample();
		void DrawColorSample( LPDRAWITEMSTRUCT lpDrawItemStruct );

											// Generated message map functions
		//{{AFX_MSG(ChPrefsColorPage)
		afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
		afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
		afx_msg void OnSelchangeComboBackColor();
		afx_msg void OnSelchangeComboTextColor();
		afx_msg void OnSelchangeComboLinkColor();
		afx_msg void OnSelchangeComboFlinkColor();
		afx_msg void OnSelchangeComboPlinkColor();
		//}}AFX_MSG

	protected:
		static const chuint32	m_luBackColors[];

		bool					m_boolInitialized;
		ChRegistry				m_regColor;

	DECLARE_MESSAGE_MAP()
};

#endif	// !defined( _CHPRFONT_H )
