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

	This file consists of the interface of the ChHTMLForm class.

----------------------------------------------------------------------------*/

// $Header$

#if (!defined( _CHHTMFRM_H ))
#define _CHHTMFRM_H


#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif



typedef struct tagChCtrlList
{
	int 		iType;						// Type of control
	char*		pstrName;					// Name of the variable
	char*		pstrValue;					// Value of the argument
	ChSize		size;						// Size of the control
	bool		boolDefSize;				// True if size not explicit
	int			iLimit;						// Limit of value
	bool		boolDefState;

	#if defined( CH_MSW )

	CWnd*		pWnd;						// Control

	#endif	// defined( CH_MSW )

} ChCtrlList, FAR *pChCtrlList;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// ChHTMLForm manager 

class ChHtmlView;

class ChHTMLForm
{
	public :
		enum tagMethod { methodPost = 1, methodGet, methodXCHCMD  };
	
		ChHTMLForm( ChHtmlView * pHtmView );
		~ChHTMLForm();

		int 	GetTotalControls() { return  m_iNumControls; }
		void 	AddControl( ChCtrlList& ctrlInfo ); 
		void 	SetNextTabFocus( CWnd* pCurrent, bool boolShiftKey );

		inline int GetAvgCharWidth() { return m_iAvgCharWidth; }
		inline int GetCharHeight() { return m_iCharHeight; }

		inline int GetEditExtraWidth() { return m_iEditExtraWidth; }
		inline int GetListExtraWidth() { return m_iListExtraWidth; }
		inline int GetPopupListExtraWidth() { return m_iPopupExtraWidth; }
		inline int GetCheckBoxWidth() { return m_sizeCheckBox.cx; }
		inline int GetCheckBoxHeight() { return m_sizeCheckBox.cy; }
		inline CFont* GetControlFont() 		{ return &m_ctrlFont; }

		pChCtrlList GetControlInfo( int iIndex )
			{
				ASSERT( iIndex < m_iNumControls );
				ASSERT( m_pList );
				return  &m_pList[iIndex];
			}
		int	GetMethod()				{ return m_iMethod; }
		const char*	GetAction()		{ return m_pstrAction; }

		void    SetMethod( int  iMethod )			{ m_iMethod = iMethod; }
		void	SetAction( const char * pstrAction ){ m_pstrAction = pstrAction; }
		void	SetMD5( const char * pstrMD5 )		{ m_strMD5 = pstrMD5; }

		#if defined( CH_MSW )
		void SubmitForm();
		void GetFormData();
		void ResetForm();
		#endif
		void AddNameValuePair( const char* pstrName, ChString& strVlaue );
	
	private :

		void EscapeSpecialChars( ChString& strData );
		ChHtmlView* GetHtmlView()		{ return m_phtmlView; }

		enum tagConst { ctrlListSize = 10, ctrlListGrowSize = 5 };
		int				m_iMethod;
		const char*		m_pstrAction;
		ChString			m_strFormData;
		ChString 			m_strMD5;
		int 			m_iNumControls;
		int				m_iCtrlListSize;
		pChCtrlList		m_pList;
		ChHtmlView*		m_phtmlView;

		int				m_iCharHeight;
		int				m_iAvgCharWidth;
		int				m_iEditExtraWidth;
		int				m_iListExtraWidth;
		int				m_iPopupExtraWidth;
		SIZE			m_sizeCheckBox;				
		CFont			m_ctrlFont;
};


#if defined( CH_MSW )

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// ChResetBtn window

class ChResetBtn : public CButton
{
// Construction
public:
	ChResetBtn();

// Attributes
public:
	ChHTMLForm *GetForm()					{ return m_pForm; }
	void SetForm(ChHTMLForm *pForm )		{ m_pForm = pForm; }

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ChResetBtn)
	//}}AFX_VIRTUAL

private :
	ChHTMLForm *m_pForm;

// Implementation
public:
	virtual ~ChResetBtn();

	// Generated message map functions
protected:
	//{{AFX_MSG(ChResetBtn)
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// ChSubmitBtn window

class ChSubmitBtn : public CButton
{
// Construction
public:
	ChSubmitBtn();

// Attributes
public:
	ChHTMLForm *GetForm()					{ return m_pForm; }
	void SetForm(ChHTMLForm *pForm )		{ m_pForm = pForm; }

// Operations
public:


private :
	ChHTMLForm *m_pForm;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ChSubmitBtn)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~ChSubmitBtn();

	// Generated message map functions
protected:
	//{{AFX_MSG(ChSubmitBtn)
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// ChFrmCombo window

class ChFrmCombo : public CComboBox
{
// Construction
public:
	ChFrmCombo();

// Attributes
public:
	ChHTMLForm *GetForm()					{ return m_pForm; }
	void SetForm(ChHTMLForm *pForm )		{ m_pForm = pForm; }

// Operations
public:

private :
	ChHTMLForm *m_pForm;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ChFrmCombo)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~ChFrmCombo();

	// Generated message map functions
protected:
	//{{AFX_MSG(ChFrmCombo)
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// ChFrmButton window

class ChFrmButton : public CButton
{
// Construction
public:
	ChFrmButton();

// Attributes
public:
	ChHTMLForm *GetForm()					{ return m_pForm; }
	void SetForm(ChHTMLForm *pForm )		{ m_pForm = pForm; }

// Operations
public:

private :
	ChHTMLForm *m_pForm;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ChFrmButton)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~ChFrmButton();
private :

	// Generated message map functions
protected:
	//{{AFX_MSG(ChFrmButton)
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// ChFrmEtxt window

class ChFrmEtxt : public CEdit
{
// Construction
public:
	ChFrmEtxt();

// Attributes
public:
	ChHTMLForm *GetForm()					{ return m_pForm; }
	void SetForm(ChHTMLForm *pForm )		{ m_pForm = pForm; }

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ChFrmEtxt)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~ChFrmEtxt();
	void 	SetMultiline( bool boolMulti ) { m_boolMultiline = boolMulti; }

private :
	ChHTMLForm *m_pForm;
	bool		m_boolMultiline;


	// Generated message map functions
protected:
	//{{AFX_MSG(ChFrmEtxt)
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// ChFrmList window

class ChFrmList : public CListBox
{
// Construction
public:
	ChFrmList();

// Attributes
public:
	ChHTMLForm *GetForm()					{ return m_pForm; }
	void SetForm(ChHTMLForm *pForm )		{ m_pForm = pForm; }


// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ChFrmList)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~ChFrmList();

private :
	ChHTMLForm *m_pForm;


	// Generated message map functions
protected:
	//{{AFX_MSG(ChFrmList)
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif // defined ( CH_MSW )

/////////////////////////////////////////////////////////////////////////////

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR    
#endif

#endif // !defined( _CHHTMFRM_H )

// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
