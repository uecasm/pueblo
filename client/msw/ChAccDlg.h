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

	This file consists of interface for the ChAccountDlg class and
	its supporting classes.  These dialogs are used to obtain new account
	information.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHACCDLG_H )
#define _CHACCDLG_H

#include <ChWizard.h>



/*----------------------------------------------------------------------------
	ChAccountDlgIntro class
----------------------------------------------------------------------------*/

class ChAccountDlgIntro : public ChWizardPage
{
	DECLARE_DYNCREATE( ChAccountDlgIntro )

	public:
		ChAccountDlgIntro();
		~ChAccountDlgIntro();

	protected:
											// Dialog Data
		//{{AFX_DATA(ChAccountDlgIntro)
		enum { IDD = IDD_ACCOUNT_INTRO };
		//}}AFX_DATA
											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChAccountDlgIntro)
		//}}AFX_VIRTUAL

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChAccountDlgIntro)
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()
};


/*----------------------------------------------------------------------------
	ChAccountDlgName class
----------------------------------------------------------------------------*/

class ChAccountDlgName : public ChWizardPage
{
	DECLARE_DYNCREATE( ChAccountDlgName )

	public:
		ChAccountDlgName();
		~ChAccountDlgName();

		void GetWizardPageData( ChString& strData );

		inline const ChString& GetFirstName() const { return m_strFirstName; }
		inline const ChString& GetSurname() const { return m_strSurname; }
		#if 0
		inline const ChString& GetMiddleName() const { return m_strMiddleInitial; }
		#endif

		virtual BOOL OnInitPage();
		virtual BOOL OnNext();

	protected:
		bool IsValid( int* piInvalidField = 0 );

	protected:
											// Dialog Data
		//{{AFX_DATA(ChAccountDlgName)
	enum { IDD = IDD_ACCOUNT_NAME };
		CString	m_strFirstName;
		CString	m_strSurname;
		CString	m_strMiddleInitial;
	CString	m_strNameFirst;
	CString	m_strNameLast;
	CString	m_strNameMiddle;
	//}}AFX_DATA
											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChAccountDlgName)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChAccountDlgName)
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()
};


/*----------------------------------------------------------------------------
	ChAccountDlgAddr class
----------------------------------------------------------------------------*/

class ChAccountDlgAddr : public ChWizardPage
{
	DECLARE_DYNCREATE( ChAccountDlgAddr )

	public:
		ChAccountDlgAddr();
		~ChAccountDlgAddr();

		#if 0
		inline const ChString& GetStreet() { return m_strStreet; }
		inline const ChString& GetApartment() { return m_strApt; }
		inline const ChString& GetCity() { return m_strCity; }
		inline const ChString& GetState() { return m_strState; }
		inline const ChString& GetZip() { return m_strZip; }
		inline const ChString& GetCountry() { return m_strCountry; }
		inline const ChString& GetEMail() { return m_strEMail; }
		#endif

		virtual BOOL OnInitPage();
		virtual BOOL OnNext();

		void GetWizardPageData( ChString& strData );
	protected:
		bool IsValid( int* piInvalidField );

	protected:
											// Dialog Data
		//{{AFX_DATA(ChAccountDlgAddr)
	enum { IDD = IDD_ACCOUNT_ADDRESS };
		CString	m_strStreet;
		CString	m_strApt;
		CString	m_strCity;
		CString	m_strState;
		CString	m_strZip;
		CString	m_strCountry;
		CString	m_strEMail;
	CString	m_strNameApt;
	CString	m_strNameCity;
	CString	m_strNameCountry;
	CString	m_strNameEMail;
	CString	m_strNameState;
	CString	m_strNameStreet;
	CString	m_strNameZip;
	//}}AFX_DATA
											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChAccountDlgAddr)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChAccountDlgAddr)
		//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};


/*----------------------------------------------------------------------------
	ChAccountDlgPersonal class
----------------------------------------------------------------------------*/

class ChAccountDlgPersonal : public ChWizardPage
{
	DECLARE_DYNCREATE( ChAccountDlgPersonal )

	public:
		ChAccountDlgPersonal();
		~ChAccountDlgPersonal();

		ChAccountInfo::Gender GetGender();
		void GetWizardPageData( ChString& strData );

		virtual BOOL OnInitPage();
		virtual BOOL OnNext();

	protected:
											/* The following enumerated type
												must match in order the 
												gender buttons in the dialog */
		typedef enum { female, male } PageGender;
		typedef enum { mmddyy, ddmmyy, yymmdd } DateFormatType;

	protected:
		bool IsValid( int* piInvalidField );
		bool GetDate( bool* pboolErrorDisplayed );
		void GetDateFormat();
	protected:
											// Dialog Data
		//{{AFX_DATA(ChAccountDlgPersonal)
		enum { IDD = IDD_ACCOUNT_PERSONAL };
		CStatic	m_staticBirthdayLabel;
		CString	m_strBirthday;
		int		m_iSex;
		//}}AFX_DATA
											// Custom data exchange
		void DDX_Sex( CDataExchange* pDX );

											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChAccountDlgPersonal)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChAccountDlgPersonal)
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()

	protected:
		ChString			m_strShortDateFormat;
		DateFormatType	m_dateFormat;

		chint16			m_sBirthDay;
		chint16			m_sBirthMonth;
		chint16			m_sBirthYear;
};


/*----------------------------------------------------------------------------
	ChAccountDlgIncome class
----------------------------------------------------------------------------*/

class ChAccountDlgIncome : public ChWizardPage
{
	DECLARE_DYNCREATE( ChAccountDlgIncome )

	public:
		ChAccountDlgIncome();
		~ChAccountDlgIncome();

		void GetWizardPageData( ChString& strData );

		virtual BOOL OnNext();

	protected:
		typedef enum { inABox, under15k, from15to25, from25to35, from35to50,
						from50to75, from75to100, from100to150, over150,
						satan, ratherNotSay } IncomeType;

	protected:
		bool IsValid( int* piInvalidField );

	protected:
											// Dialog Data
		//{{AFX_DATA(ChAccountDlgIncome)
		enum { IDD = IDD_ACCOUNT_INCOME };
		int		m_iIncomeCategory;
		//}}AFX_DATA
											// Custom data exchange
		void DDX_Sex( CDataExchange* pDX );

											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChAccountDlgIncome)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChAccountDlgIncome)
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()
};


/*----------------------------------------------------------------------------
	ChAccountDlgConn class
----------------------------------------------------------------------------*/

class ChAccountDlgConn : public ChWizardPage
{
	DECLARE_DYNCREATE( ChAccountDlgConn )

	public:
		ChAccountDlgConn();
		~ChAccountDlgConn();

		virtual BOOL OnInitPage();
		void GetWizardPageData( ChString& strData );
											// Dialog Data
		//{{AFX_DATA(ChAccountDlgConn)
		enum { IDD = IDD_ACCOUNT_CONNECTION };
		int		m_iConnSpeed;
		int		m_iConnType;
		//}}AFX_DATA
											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChAccountDlgConn)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
											/* The following enumerated types
												must match in order the 
												corresponding buttons in the
												dialog */

		typedef enum { direct, school, aol, compuserve, prodigy,
						other } ConnType;
		typedef enum { modem9600, modem144, modem288, modem336, modem56,
						isdn, fractT1, t1 } ConnSpeed;

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChAccountDlgConn)
		// NOTE: the ClassWizard will add member functions here
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()
};


/*----------------------------------------------------------------------------
	ChAccountDlgSurvey1 class
----------------------------------------------------------------------------*/

class ChAccountDlgSurvey1 : public ChWizardPage
{
	DECLARE_DYNCREATE( ChAccountDlgSurvey1 )

	public:
		ChAccountDlgSurvey1();
		~ChAccountDlgSurvey1();

		virtual BOOL OnInitPage();

		void GetWizardPageData( ChString& strData );
											// Dialog Data
		//{{AFX_DATA(ChAccountDlgSurvey1)
		enum { IDD = IDD_ACCOUNT_SURVEY1 };
		CEdit	m_editAdditionalInfo;
		CString	m_strHow;
		int		m_iHowFoundPueblo;
		CString	m_strAdditionalInfo;
		//}}AFX_DATA

											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChAccountDlgSurvey1)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
											/* The following enumerated type
												must match in order the 
												corresponding buttons in the
												dialog */

		typedef enum { webPointer, magazine, aFriend, newsgroup, mailingList,
						other } DiscoveryType;

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChAccountDlgSurvey1)
		afx_msg void OnRadioFriend();
		afx_msg void OnRadioListArticle();
		afx_msg void OnRadioMagArticle();
		afx_msg void OnRadioNewsArticle();
		afx_msg void OnRadioOther();
		afx_msg void OnRadioPointer();
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()
};


/*----------------------------------------------------------------------------
	ChAccountDlgSurvey2 class
----------------------------------------------------------------------------*/

class ChAccountDlgSurvey2 : public ChWizardPage
{
	DECLARE_DYNCREATE( ChAccountDlgSurvey2 )

	public:
		ChAccountDlgSurvey2();
		~ChAccountDlgSurvey2();

		void GetWizardPageData( ChString& strData );
											// Dialog Data
		//{{AFX_DATA(ChAccountDlgSurvey2)
		enum { IDD = IDD_ACCOUNT_SURVEY2 };
		int		m_iAdsOpinion;
		//}}AFX_DATA

											/* ClassWizard generate virtual
												function overrides */
		//{{AFX_VIRTUAL(ChAccountDlgSurvey2)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
											/* The following enumerated type
												must match in order the 
												corresponding buttons in the
												dialog */

		typedef enum { adsOkay, yesPay, noPay } AdQuestionType;

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChAccountDlgSurvey2)
		// NOTE: the ClassWizard will add member functions here
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()
};


/*----------------------------------------------------------------------------
	ChAccountDlg class
----------------------------------------------------------------------------*/

class ChAccountDlg : public ChWizard
{
	DECLARE_DYNAMIC( ChAccountDlg )

	public:
		ChAccountDlg( CWnd* pParentWnd = 0 );
		virtual ~ChAccountDlg();

		inline const ChAccountDlgName* GetNamePage() const { return &namePage; }

		//void GetInfo( ChAccountInfo& info );

		void GetWizardData( ChString& strData );
											/* ClassWizard generated virtual
												function overrides */
		//{{AFX_VIRTUAL(ChAccountDlg)
		//}}AFX_VIRTUAL

	protected:
		void AddPages();

	protected:
		//{{AFX_MSG(ChAccountDlg)
		//}}AFX_MSG

	protected:
		ChAccountDlgIntro		introPage;
		ChAccountDlgName		namePage;
		ChAccountDlgAddr		addressPage;
		ChAccountDlgPersonal	personalPage;
		ChAccountDlgIncome		incomePage;
		ChAccountDlgConn		connectionPage;
		ChAccountDlgSurvey1		survey1Page;
		ChAccountDlgSurvey2		survey2Page;

		DECLARE_MESSAGE_MAP()
};

#endif	// !defined( _CHACCDLG_H )

// $Log$
