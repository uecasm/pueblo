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

	This file consists of implementations of the ChAccountDlg class and
	its supporting classes.  These dialogs are used to obtain new account
	information.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"
#include "Pueblo.h"
#include "resource.h"

#include "ChAccDlg.h"
#include "MemDebug.h"

#if defined( _DEBUG )

	#undef THIS_FILE
	static char BASED_CODE THIS_FILE[] = __FILE__;

#endif	// defined( _DEBUG )


/*----------------------------------------------------------------------------
	Utility function forward declarations
----------------------------------------------------------------------------*/

											/* Determine number of elements
												in an array (not bytes) */

#ifndef _countof
#define _countof( array )	(sizeof( array ) / sizeof( array[0] ))
#endif

/*----------------------------------------------------------------------------
	ChAccountDlg class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNAMIC( ChAccountDlg, ChWizard )

ChAccountDlg::ChAccountDlg( CWnd* pParentWnd ) :
					ChWizard( (chparam)AfxGetApp()->m_hInstance,
								IDS_ACCOUNT_WIZARD, pParentWnd )
{
	AddPages();
}


ChAccountDlg::~ChAccountDlg()
{
}


BEGIN_MESSAGE_MAP( ChAccountDlg, ChWizard )
	//{{AFX_MSG_MAP(ChAccountDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


#if 0
/*----------------------------------------------------------------------------
	ChAccountDlg public methods
----------------------------------------------------------------------------*/

void ChAccountDlg::GetInfo( ChAccountInfo& info )
{
	ChString		strMiscData;

	info.SetName( namePage.GetFirstName(), namePage.GetMiddleName(),
					namePage.GetSurname() );

	info.SetAddress( addressPage.GetStreet(), addressPage.GetApartment(),
						addressPage.GetCity(), addressPage.GetState(),
						addressPage.GetZip(), addressPage.GetCountry(),
						addressPage.GetEMail() );

	info.SetPersonal( personalPage.GetGender() );

	personalPage.GetData( info );
	incomePage.GetData( info );
	connectionPage.GetData( info );
	survey1Page.GetData( info );
	survey2Page.GetData( info );
}
#else
/*----------------------------------------------------------------------------
	ChAccountDlg public methods
----------------------------------------------------------------------------*/

void ChAccountDlg::GetWizardData( ChString& strPageData )
{
	namePage.GetWizardPageData(  strPageData );
	addressPage.GetWizardPageData(  strPageData );
	personalPage.GetWizardPageData( strPageData );
	incomePage.GetWizardPageData( strPageData );
	connectionPage.GetWizardPageData( strPageData );
	survey1Page.GetWizardPageData( strPageData );
	survey2Page.GetWizardPageData( strPageData );
}

#endif


/*----------------------------------------------------------------------------
	ChAccountDlg protected methods
----------------------------------------------------------------------------*/

void ChAccountDlg::AddPages()
{
	this->AddPage( &introPage );
	this->AddPage( &namePage );
	this->AddPage( &addressPage );
	this->AddPage( &personalPage );
	this->AddPage( &incomePage );
	this->AddPage( &connectionPage );
	this->AddPage( &survey1Page );
	this->AddPage( &survey2Page );
}


/*----------------------------------------------------------------------------
	ChAccountDlgIntro class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChAccountDlgIntro, ChWizardPage )

ChAccountDlgIntro::ChAccountDlgIntro() :
					ChWizardPage( ChAccountDlgIntro::IDD )
{
	//{{AFX_DATA_INIT(ChAccountDlgIntro)
	//}}AFX_DATA_INIT
}

ChAccountDlgIntro::~ChAccountDlgIntro()
{
}


BEGIN_MESSAGE_MAP( ChAccountDlgIntro, ChWizardPage )
	//{{AFX_MSG_MAP(ChAccountDlgIntro)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChAccountDlgName class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChAccountDlgName, ChWizardPage )

ChAccountDlgName::ChAccountDlgName() :
					ChWizardPage( ChAccountDlgName::IDD )
{
	//{{AFX_DATA_INIT(ChAccountDlgName)
	m_strFirstName = _T("");
	m_strSurname = _T("");
	m_strMiddleInitial = _T("");
	m_strNameFirst = _T("first");
	m_strNameLast = _T("last");
	m_strNameMiddle = _T("middle");
	//}}AFX_DATA_INIT
}

ChAccountDlgName::~ChAccountDlgName()
{
}


BOOL ChAccountDlgName::OnInitPage()
{
	ChWizardPage::OnInitPage();

	GetDlgItem( IDC_EDIT_FIRST_NAME )->SetFocus();

	((CEdit*)GetDlgItem( IDC_EDIT_FIRST_NAME ))->LimitText( 30 );
	((CEdit*)GetDlgItem( IDC_EDIT_MIDDLE_NAME ))->LimitText( 2 );
	((CEdit*)GetDlgItem( IDC_EDIT_LAST_NAME ))->LimitText( 30 );

	return false;
}


BOOL ChAccountDlgName::OnNext()
{
	int		iInvalidField;
	bool	boolValid = IsValid( &iInvalidField );

	if (!boolValid)
	{
		CWnd*	pItem = GetDlgItem( iInvalidField );

		AfxMessageBox( IDS_ACCOUNT_NAME_ERROR );
		if (pItem)
		{
			pItem->SetFocus();
		}
	}

	return boolValid;
}


void ChAccountDlgName::DoDataExchange( CDataExchange* pDX )
{
	ChWizardPage::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChAccountDlgName)
	DDX_Text(pDX, IDC_EDIT_FIRST_NAME, m_strFirstName);
	DDX_Text(pDX, IDC_EDIT_LAST_NAME, m_strSurname);
	DDX_Text(pDX, IDC_EDIT_MIDDLE_NAME, m_strMiddleInitial);
	//}}AFX_DATA_MAP

/*
	if ( pDX->m_bSaveAndValidate  ) 
	{
		DDX_Text(pDX, IDC_NAME_FIRST, m_strNameFirst);
		DDX_Text(pDX, IDC_NAME_LAST, m_strNameLast);
		DDX_Text(pDX, IDC_NAME_MIDDLE, m_strNameMiddle);
	}
*/
}

void ChAccountDlgName::GetWizardPageData( ChString& strData )
{
	UpdateNameValue( strData, m_strNameFirst, m_strFirstName );
	UpdateNameValue( strData, m_strNameLast, m_strSurname );
	UpdateNameValue( strData, m_strNameMiddle, m_strMiddleInitial );

}


BEGIN_MESSAGE_MAP( ChAccountDlgName, ChWizardPage )
	//{{AFX_MSG_MAP(ChAccountDlgName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChAccountDlgName protected methods
----------------------------------------------------------------------------*/

bool ChAccountDlgName::IsValid( int* piInvalidField )
{
	bool	boolValid = true;
	int		iTemp;

	UpdateData();

	if (0 == piInvalidField)
	{
		piInvalidField = &iTemp;
	}
	*piInvalidField = 0;

	if (m_strFirstName.IsEmpty())
	{
		*piInvalidField = IDC_EDIT_FIRST_NAME;
		boolValid = false;
	}
	else if (m_strSurname.IsEmpty())
	{
		*piInvalidField = IDC_EDIT_LAST_NAME;
		boolValid = false;
	}

	return boolValid;
}


/*----------------------------------------------------------------------------
	ChAccountDlgName message handlers
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
	ChAccountDlgAddr class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChAccountDlgAddr, ChWizardPage )

ChAccountDlgAddr::ChAccountDlgAddr() :
					ChWizardPage( ChAccountDlgAddr::IDD )
{
	//{{AFX_DATA_INIT(ChAccountDlgAddr)
	m_strStreet = _T("");
	m_strApt = _T("");
	m_strCity = _T("");
	m_strState = _T("");
	m_strZip = _T("");
	m_strCountry = _T("");
	m_strEMail = _T("");
	m_strNameApt = _T("apt");
	m_strNameCity = _T("city");
	m_strNameCountry = _T("country");
	m_strNameEMail = _T("email");
	m_strNameState = _T("state");
	m_strNameStreet = _T("street");
	m_strNameZip = _T("zip");
	//}}AFX_DATA_INIT
}


ChAccountDlgAddr::~ChAccountDlgAddr()
{
}


BOOL ChAccountDlgAddr::OnInitPage()
{
	ChWizardPage::OnInitPage();

	GetDlgItem( IDC_EDIT_STREET )->SetFocus();

	((CEdit*)GetDlgItem( IDC_EDIT_STREET ))->LimitText( 60 );
	((CEdit*)GetDlgItem( IDC_EDIT_APT ))->LimitText( 8 );
	((CEdit*)GetDlgItem( IDC_EDIT_CITY ))->LimitText( 30 );
	((CEdit*)GetDlgItem( IDC_EDIT_STATE ))->LimitText( 16 );
	((CEdit*)GetDlgItem( IDC_EDIT_ZIP ))->LimitText( 10 );
	((CEdit*)GetDlgItem( IDC_EDIT_COUNTRY ))->LimitText( 32 );
	((CEdit*)GetDlgItem( IDC_EDIT_EMAIL ))->LimitText( 255 );

	return false;
}


BOOL ChAccountDlgAddr::OnNext()
{
	int		iInvalidField;
	bool	boolValid = IsValid( &iInvalidField );

	if (!boolValid)
	{
		CWnd*	pItem = GetDlgItem( iInvalidField );

		AfxMessageBox( IDS_ACCOUNT_ADDRESS_ERROR );
		if (pItem)
		{
			pItem->SetFocus();
		}
	}

	return boolValid;
}


void ChAccountDlgAddr::DoDataExchange( CDataExchange* pDX )
{
	ChWizardPage::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChAccountDlgAddr)
	DDX_Text( pDX, IDC_EDIT_STREET, m_strStreet );
	DDX_Text(pDX, IDC_EDIT_APT, m_strApt);
	DDX_Text( pDX, IDC_EDIT_CITY, m_strCity );
	DDX_Text( pDX, IDC_EDIT_STATE, m_strState );
	DDX_Text( pDX, IDC_EDIT_ZIP, m_strZip );
	DDX_Text(pDX, IDC_EDIT_COUNTRY, m_strCountry);
	DDX_Text(pDX, IDC_EDIT_EMAIL, m_strEMail);
	//}}AFX_DATA_MAP

/*
	if ( pDX->m_bSaveAndValidate  ) 
	{
		DDX_Text(pDX, IDC_NAME_APT, m_strNameApt);
		DDX_Text(pDX, IDC_NAME_CITY, m_strNameCity);
		DDX_Text(pDX, IDC_NAME_COUNTRY, m_strNameCountry);
		DDX_Text(pDX, IDC_NAME_EMAIL, m_strNameEMail);
		DDX_Text(pDX, IDC_NAME_STATE, m_strNameState);
		DDX_Text(pDX, IDC_NAME_STREET, m_strNameStreet);
		DDX_Text(pDX, IDC_NAME_ZIP, m_strNameZip);
	}
*/
}

void ChAccountDlgAddr::GetWizardPageData( ChString& strData )
{
	UpdateNameValue( strData, m_strNameStreet, m_strStreet );
	UpdateNameValue( strData, m_strNameApt, m_strApt );
	UpdateNameValue( strData, m_strNameCity, m_strCity );
	UpdateNameValue( strData, m_strNameState, m_strState );
	UpdateNameValue( strData, m_strNameZip, m_strZip );
	UpdateNameValue( strData, m_strNameCountry, m_strCountry );
	UpdateNameValue( strData, m_strNameEMail, m_strEMail );

}



BEGIN_MESSAGE_MAP( ChAccountDlgAddr, ChWizardPage )
	//{{AFX_MSG_MAP(ChAccountDlgAddr)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChAccountDlgAddr protected methods
----------------------------------------------------------------------------*/

bool ChAccountDlgAddr::IsValid( int* piInvalidField )
{
	bool	boolValid = true;
	int		iTemp;

	UpdateData();

	if (0 == piInvalidField)
	{
		piInvalidField = &iTemp;
	}
	*piInvalidField = 0;

	/*if (m_strStreet.IsEmpty())
	{
		*piInvalidField = IDC_EDIT_STREET;
		boolValid = false;
	}
	else*/ if (m_strCity.IsEmpty())
	{
		*piInvalidField = IDC_EDIT_CITY;
		boolValid = false;
	}
	else if (m_strState.IsEmpty())
	{
		*piInvalidField = IDC_EDIT_STATE;
		boolValid = false;
	}
	else if (m_strZip.IsEmpty())
	{
		*piInvalidField = IDC_EDIT_ZIP;
		boolValid = false;
	}

	return boolValid;
}


/*----------------------------------------------------------------------------
	ChAccountDlgAddr message handlers
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
	ChAccountDlgPersonal class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChAccountDlgPersonal, ChWizardPage )

ChAccountDlgPersonal::ChAccountDlgPersonal() :
					ChWizardPage( ChAccountDlgPersonal::IDD )
{
	m_iSex = female;

	//{{AFX_DATA_INIT( ChAccountDlgPersonal )
	m_strBirthday = _T("");
	//}}AFX_DATA_INIT

	GetDateFormat();
}


ChAccountDlgPersonal::~ChAccountDlgPersonal()
{
}


ChAccountInfo::Gender ChAccountDlgPersonal::GetGender()
{
	ChAccountInfo::Gender	gender;

	if (m_iSex == female)
	{
		gender = ChAccountInfo::female;
	}
	else if (m_iSex == male)
	{
		gender = ChAccountInfo::male;
	}
	else
	{
		gender = ChAccountInfo::other;
	}

	return gender;
}



BOOL ChAccountDlgPersonal::OnInitPage()
{
	ChString		strFormat;
	ChString		strLabel;

	ChWizardPage::OnInitPage();

	GetDlgItem( IDC_EDIT_BIRTHDATE )->SetFocus();

	((CEdit*)GetDlgItem( IDC_EDIT_BIRTHDATE ))->LimitText( 10 );

	m_staticBirthdayLabel.GetWindowText( strFormat );
	strLabel.Format( strFormat, (const char*)m_strShortDateFormat );
	m_staticBirthdayLabel.SetWindowText( strLabel );

	return false;
}


BOOL ChAccountDlgPersonal::OnNext()
{
	int		iInvalidField;
	bool	boolValid = IsValid( &iInvalidField );

											/* IsValid() for this class
												displays a message if there
												is an error */
	if (!boolValid)
	{
		CWnd*	pItem = GetDlgItem( iInvalidField );

		if (pItem)
		{
			pItem->SetFocus();
		}
	}

	return boolValid;
}


void ChAccountDlgPersonal::DoDataExchange( CDataExchange* pDX )
{
	ChWizardPage::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChAccountDlgPersonal)
	DDX_Control(pDX, IDC_BIRTHDAY_LABEL, m_staticBirthdayLabel);
	DDX_Text(pDX, IDC_EDIT_BIRTHDATE, m_strBirthday);
	DDX_Radio(pDX, IDC_RADIO_SEX_FEMALE, m_iSex);
	//}}AFX_DATA_MAP
}

void ChAccountDlgPersonal::GetWizardPageData( ChString& strData )
{
	char	buffer[16];

	sprintf( buffer, "%d-%d-%d", m_sBirthYear, m_sBirthMonth, m_sBirthDay );
 	UpdateNameValue( strData, "birthdate", buffer );

  UpdateNameValue( strData, "birth_raw", m_strBirthday );

	ChString strGender;
	if (m_iSex == female)
	{
		strGender = TEXT( "female");
	}
	else if (m_iSex == male)
	{
		strGender = TEXT( "male");
	}
	else
	{
		strGender = TEXT( "other");
	}
	UpdateNameValue( strData, "Gender", strGender );

}



BEGIN_MESSAGE_MAP( ChAccountDlgPersonal, ChWizardPage )
	//{{AFX_MSG_MAP(ChAccountDlgPersonal)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChAccountDlgPersonal protected methods
----------------------------------------------------------------------------*/

bool ChAccountDlgPersonal::IsValid( int* piInvalidField )
{
	bool	boolValid = true;
	bool	boolErrorDisplayed = false;
	int		iTemp;

	if (0 == piInvalidField)
	{
		piInvalidField = &iTemp;
	}
	*piInvalidField = 0;

	UpdateData();

	if (m_strBirthday.IsEmpty() || !GetDate( &boolErrorDisplayed ))
	{
		if (!boolErrorDisplayed)
		{
			ChString		strFormat;
			ChString		strMessage;

			LOADSTRING( IDS_ACCOUNT_BIRTHDAY_ERROR, strFormat );
			strMessage.Format( strFormat, m_strShortDateFormat );

			AfxMessageBox( strMessage );
		}

		*piInvalidField = IDC_EDIT_BIRTHDATE;
		boolValid = false;
	}

	return boolValid;
}


void ChAccountDlgPersonal::GetDateFormat()
{
	char	buffer[100];
	int		iDateFormat;

	::GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_IDATE,
						buffer, sizeof( buffer ) );
	iDateFormat = atoi( buffer );
	if ((iDateFormat < 0) || (iDateFormat > 2))
	{
		iDateFormat = 0;
	}
	m_dateFormat = (DateFormatType)iDateFormat;

	switch (m_dateFormat)
	{
		case mmddyy:
		{
			m_strShortDateFormat = "mm/dd/yyyy";
			break;
		}

		case ddmmyy:
		{
			m_strShortDateFormat = "dd/mm/yyyy";
			break;
		}

		case yymmdd:
		{
			m_strShortDateFormat = "yyyy/mm/dd";
			break;
		}

		default:
		{
			break;
		}
	}
}


bool ChAccountDlgPersonal::GetDate( bool* pboolErrorDisplayed )
{
	static int		iDaysInMonth[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30,
										31, 30, 31 };
	bool			boolValid = false;
	const char*		pstrBirthday = m_strBirthday;
	int				iCount = 0;
	int				iFirst = 0;
	int				iSecond = 0;
	int				iThird = 0;
	int*			piWorking = &iFirst;
	int				iMonth;
	int				iDay;
	int				iYear;
	ChTime			rightNow( ChTime::GetCurrentTime() );

	ASSERT( pboolErrorDisplayed );
	*pboolErrorDisplayed = false;

	while (*pstrBirthday && !isdigit( *pstrBirthday ))
	{
		pstrBirthday++;
	}

	if (isdigit( *pstrBirthday ))
	{
		while (*pstrBirthday && (iCount < 3))
		{
			while (isdigit( *pstrBirthday ))
			{
				*piWorking *= 10;
				*piWorking += (*pstrBirthday - '0');
				pstrBirthday++;
			}

			while (*pstrBirthday && !isdigit( *pstrBirthday ))
			{
				pstrBirthday++;
			}

			iCount++;
			if (1 == iCount)
			{
				piWorking = &iSecond;
			}
			else if (2 == iCount)
			{
				piWorking = &iThird;
			}
		}
	}

	if ((3 == iCount) && iFirst && iSecond && iThird)
	{
		switch( m_dateFormat )
		{
			case mmddyy:
			{
				iMonth = iFirst;
				iDay = iSecond;
				iYear = iThird;
				break;
			}

			case ddmmyy:
			{
				iDay = iFirst;
				iMonth = iSecond;
				iYear = iThird;
				break;
			}

			case yymmdd:
			{
				iYear = iFirst;
				iMonth = iSecond;
				iDay = iThird;
				break;
			}
		}

		if ((iYear >= 1800) && (iYear < rightNow.GetYear()))
		{
			if ((iMonth >= 1) && (iMonth <= 12))
			{
				int		iMaxDays = iDaysInMonth[iMonth - 1];

				if ((iMonth >= 1) && (iMonth <= iMaxDays))
				{
					ChTime	birthDay( (chint16)iYear, (chint16)iMonth - 1,
										(chint16)iDay, 0, 0, 0 );
					ChTime	compareDate( rightNow.GetYear() - 2,
											rightNow.GetMonth(),
											rightNow.GetDay(),
											rightNow.GetHour(),
											rightNow.GetMinute(),
											rightNow.GetSecond() );
					ChTimeSpan	spanAge;
					ChTimeSpan	spanCompare;
					chint32		lYearsOld;

					spanAge = rightNow - birthDay;
					spanCompare = rightNow - compareDate;

					lYearsOld = spanAge.GetDays();
					lYearsOld = (chint32)((float)lYearsOld / 365.25);

					if (spanAge < spanCompare)
					{
						ChString		strFormat;
						ChString		strMessage;

						LOADSTRING( IDS_ACCOUNT_AGE_TOO_YOUNG, strMessage );

						AfxMessageBox( strMessage );

						boolValid = false;
						*pboolErrorDisplayed = true;
					}
					else if (lYearsOld > 125)
					{
						ChString		strFormat;
						ChString		strMessage;

						LOADSTRING( IDS_ACCOUNT_AGE_TOO_OLD, strFormat );
						strMessage.Format( strFormat, (int)lYearsOld );

						AfxMessageBox( strMessage );

						boolValid = false;
						*pboolErrorDisplayed = true;
					}
					else
					{						// This date appears to be valid
						boolValid = true;
						m_sBirthMonth = (chint16)iMonth;
						m_sBirthDay = (chint16)iDay;
						m_sBirthYear = (chint16)iYear;
					}
				}
			}
		}
	}

	return boolValid;
}


/*----------------------------------------------------------------------------
	ChAccountDlgIncome class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChAccountDlgIncome, ChWizardPage )

ChAccountDlgIncome::ChAccountDlgIncome() :
					ChWizardPage( ChAccountDlgIncome::IDD )
{
	m_iIncomeCategory = (int)inABox;

	//{{AFX_DATA_INIT( ChAccountDlgIncome )
	//}}AFX_DATA_INIT
}


ChAccountDlgIncome::~ChAccountDlgIncome()
{
}


BOOL ChAccountDlgIncome::OnNext()
{
	int		iInvalidField;
	bool	boolValid = IsValid( &iInvalidField );

											/* IsValid() for this class
												displays a message if there
												is an error */
	if (!boolValid)
	{
		CWnd*	pItem = GetDlgItem( iInvalidField );

		if (pItem)
		{
			pItem->SetFocus();
		}
	}

	return boolValid;
}


void ChAccountDlgIncome::DoDataExchange( CDataExchange* pDX )
{
	ChWizardPage::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChAccountDlgIncome)
	DDX_Radio(pDX, IDC_RADIO_IN_A_BOX, m_iIncomeCategory);
	//}}AFX_DATA_MAP
}

void ChAccountDlgIncome::GetWizardPageData( ChString& strData )
{
	static char*	pstrFields[] =	{	"0-0",
										"0-15k",
										"15k-25k",
										"25k-35k",
										"35k-50k",
										"50k-75k",
										"75k-100k",
										"100k-150k",
										"150k-1000m",
										"1000m-1000m",
										"" };
	ChString			strIncome = pstrFields[m_iIncomeCategory];

 	UpdateNameValue( strData, "income", strIncome );
}



BEGIN_MESSAGE_MAP( ChAccountDlgIncome, ChWizardPage )
	//{{AFX_MSG_MAP(ChAccountDlgIncome)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChAccountDlgIncome protected methods
----------------------------------------------------------------------------*/

bool ChAccountDlgIncome::IsValid( int* piInvalidField )
{
	IncomeType	income;
	bool		boolValid;
	int			iTemp;

	if (0 == piInvalidField)
	{
		piInvalidField = &iTemp;
	}
	*piInvalidField = 0;

	UpdateData();

	income = (IncomeType)m_iIncomeCategory;
	switch( income )
	{
		case inABox:
		{
			AfxMessageBox( IDS_INCOME_NOT_A_BOX );
			*piInvalidField = IDC_RADIO_IN_A_BOX;
			boolValid = false;
			break;
		}

		case satan:
		{
			ChAccountDlg*	pParent = (ChAccountDlg*)GetParent();
			ChString			strFirstName;
			ChString			strMiddleName;
			ChString			strSurname;

			if (pParent)
			{
				const ChAccountDlgName*	pDlgName = pParent->GetNamePage();

				if (pDlgName)
				{
					strFirstName = pDlgName->GetFirstName();
					strSurname = pDlgName->GetSurname();
				}
			}

			if (strFirstName.IsEmpty() && strSurname.IsEmpty())
			{
				strFirstName = "some weirdly unnamed";
				strSurname = "person";
			}

			if (((0 == strcmpi( strFirstName, "bill" )) ||
					(0 == strcmpi( strFirstName, "william" ))) &&
				(0 == strcmpi( strSurname, "gates" )))
			{
				AfxMessageBox( IDS_INCOME_IS_BILL_GATES );
				boolValid = true;
			}
			else
			{
				ChString		strFormat;
				ChString		strOut;

				LOADSTRING( IDS_INCOME_NOT_BILL_GATES, strFormat );
				strOut.Format( strFormat, (const char*)strFirstName,
									(const char*)strSurname );

				AfxMessageBox( strOut );
				*piInvalidField = IDC_RADIO_BILL_GATES;
				boolValid = false;
			}
			break;
		}

		default:
		{
			boolValid = true;
			break;
		}
	}

	return boolValid;
}


/*----------------------------------------------------------------------------
	ChAccountDlgConn class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChAccountDlgConn, ChWizardPage )

ChAccountDlgConn::ChAccountDlgConn() :
						ChWizardPage( ChAccountDlgConn::IDD )
{
	m_iConnType = direct;
	m_iConnSpeed = modem9600;

	//{{AFX_DATA_INIT(ChAccountDlgConn)
	//}}AFX_DATA_INIT
}


ChAccountDlgConn::~ChAccountDlgConn()
{
}


BOOL ChAccountDlgConn::OnInitPage()
{
	ChWizardPage::OnInitPage();

	CComboBox *ctlCombo;
	ChString strText;
	
	ctlCombo = (CComboBox *)GetDlgItem(IDC_COMBO_CONN_HOW);
	ctlCombo->ResetContent();
	LOADSTRING(IDS_ACCOUNT_DIRECT, strText);
	ctlCombo->AddString(strText);
	LOADSTRING(IDS_ACCOUNT_UNI, strText);
	ctlCombo->AddString(strText);
	LOADSTRING(IDS_ACCOUNT_PRODIGY, strText);
	ctlCombo->AddString(strText);
	LOADSTRING(IDS_ACCOUNT_AOL, strText);
	ctlCombo->AddString(strText);
	LOADSTRING(IDS_ACCOUNT_COMPUSERVE, strText);
	ctlCombo->AddString(strText);
	LOADSTRING(IDS_ACCOUNT_OTHER, strText);
	ctlCombo->AddString(strText);
	ctlCombo->SetExtendedUI();
	ctlCombo->SetCurSel(m_iConnType);

	ctlCombo = (CComboBox *)GetDlgItem(IDC_COMBO_CONN_SPEED);
	ctlCombo->ResetContent();
	LOADSTRING(IDS_ACCOUNT_9600, strText);
	ctlCombo->AddString(strText);
	LOADSTRING(IDS_ACCOUNT_14400, strText);
	ctlCombo->AddString(strText);
	LOADSTRING(IDS_ACCOUNT_28800, strText);
	ctlCombo->AddString(strText);
	LOADSTRING(IDS_ACCOUNT_33600, strText);
	ctlCombo->AddString(strText);
	LOADSTRING(IDS_ACCOUNT_56000, strText);
	ctlCombo->AddString(strText);
	LOADSTRING(IDS_ACCOUNT_DSL, strText);
	ctlCombo->AddString(strText);
	LOADSTRING(IDS_ACCOUNT_SAT, strText);
	ctlCombo->AddString(strText);
	LOADSTRING(IDS_ACCOUNT_ISDN, strText);
	ctlCombo->AddString(strText);
	LOADSTRING(IDS_ACCOUNT_PARTT1, strText);
	ctlCombo->AddString(strText);
	LOADSTRING(IDS_ACCOUNT_T1, strText);
	ctlCombo->AddString(strText);
	LOADSTRING(IDS_ACCOUNT_UNKNOWN, strText);
	ctlCombo->AddString(strText);
	ctlCombo->SetExtendedUI();
	ctlCombo->SetCurSel(m_iConnSpeed);

	return false;
}


void ChAccountDlgConn::GetWizardPageData( ChString& strData )
{
	static char*	pstrConnTypes[] = {	"direct",
										"university",
										"aol",
										"compuserve",
										"prodigy",
										"other" };
	static char*	pstrConnSpeed[] = {	"9600",
										"14400",
										"28800",
										"33600",
										"56000",
										"dsl",
										"sat",
										"isdn",
										"fractional t1",
										"t1" };

 	UpdateNameValue( strData, "connection_type", pstrConnTypes[m_iConnType] );
 	UpdateNameValue( strData, "connection_speed", pstrConnSpeed[m_iConnSpeed] );
}




void ChAccountDlgConn::DoDataExchange( CDataExchange* pDX )
{
	ChWizardPage::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(ChAccountDlgConn)
	DDX_CBIndex(pDX, IDC_COMBO_CONN_SPEED, m_iConnSpeed);
	DDX_CBIndex(pDX, IDC_COMBO_CONN_HOW, m_iConnType);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( ChAccountDlgConn, ChWizardPage )
	//{{AFX_MSG_MAP(ChAccountDlgConn)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChAccountDlgSurvey1 class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChAccountDlgSurvey1, ChWizardPage )

ChAccountDlgSurvey1::ChAccountDlgSurvey1() :
						ChWizardPage( ChAccountDlgSurvey1::IDD )
{
	m_iHowFoundPueblo = webPointer;
	LOADSTRING( IDS_SURVEY_WHICH_SITE, m_strHow );

	//{{AFX_DATA_INIT(ChAccountDlgSurvey1)
	m_strAdditionalInfo = _T("");
	//}}AFX_DATA_INIT
}


ChAccountDlgSurvey1::~ChAccountDlgSurvey1()
{
}


BOOL ChAccountDlgSurvey1::OnInitPage()
{
	ChWizardPage::OnInitPage();

	m_editAdditionalInfo.LimitText( 100 );

	return false;
}



void ChAccountDlgSurvey1::GetWizardPageData( ChString& strData )
{
	static char*	pstrRefTypes[] = {	"web",
										"magazine",
										"friend",
										"newsgroup",
										"mailing list",
										"other" };

 	UpdateNameValue( strData, "reference", pstrRefTypes[m_iHowFoundPueblo]  );
 	UpdateNameValue( strData, "reference_info", m_strAdditionalInfo );
}



void ChAccountDlgSurvey1::DoDataExchange(CDataExchange* pDX)
{
	ChWizardPage::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChAccountDlgSurvey1)
	DDX_Control(pDX, IDC_EDIT_ADDITIONAL_INFO, m_editAdditionalInfo);
	DDX_Text(pDX, IDC_STATIC_HOW, m_strHow);
	DDX_Radio(pDX, IDC_RADIO_POINTER, m_iHowFoundPueblo);
	DDX_Text(pDX, IDC_EDIT_ADDITIONAL_INFO, m_strAdditionalInfo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( ChAccountDlgSurvey1, ChWizardPage )
	//{{AFX_MSG_MAP(ChAccountDlgSurvey1)
	ON_BN_CLICKED(IDC_RADIO_FRIEND, OnRadioFriend)
	ON_BN_CLICKED(IDC_RADIO_LIST_ARTICLE, OnRadioListArticle)
	ON_BN_CLICKED(IDC_RADIO_MAG_ARTICLE, OnRadioMagArticle)
	ON_BN_CLICKED(IDC_RADIO_NEWS_ARTICLE, OnRadioNewsArticle)
	ON_BN_CLICKED(IDC_RADIO_OTHER, OnRadioOther)
	ON_BN_CLICKED(IDC_RADIO_POINTER, OnRadioPointer)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChAccountDlgSurvey1 message handlers
----------------------------------------------------------------------------*/

void ChAccountDlgSurvey1::OnRadioPointer() 
{
	UpdateData();
	LOADSTRING( IDS_SURVEY_WHICH_SITE, m_strHow );
	UpdateData( false );
}

void ChAccountDlgSurvey1::OnRadioFriend() 
{
	UpdateData();
	LOADSTRING( IDS_SURVEY_WHICH_FRIEND, m_strHow );
	UpdateData( false );
}

void ChAccountDlgSurvey1::OnRadioListArticle() 
{
	UpdateData();
	LOADSTRING( IDS_SURVEY_WHICH_LIST, m_strHow );
	UpdateData( false );
}

void ChAccountDlgSurvey1::OnRadioMagArticle() 
{
	UpdateData();
	LOADSTRING( IDS_SURVEY_WHICH_MAG, m_strHow );
	UpdateData( false );
}

void ChAccountDlgSurvey1::OnRadioNewsArticle() 
{
	UpdateData();
	LOADSTRING( IDS_SURVEY_WHICH_NEWS, m_strHow );
	UpdateData( false );
}

void ChAccountDlgSurvey1::OnRadioOther() 
{
	UpdateData();
	LOADSTRING( IDS_SURVEY_WHAT, m_strHow );
	UpdateData( false );
}


/*----------------------------------------------------------------------------
	ChAccountDlgSurvey2 class
----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE( ChAccountDlgSurvey2, ChWizardPage )

ChAccountDlgSurvey2::ChAccountDlgSurvey2() :
						ChWizardPage( ChAccountDlgSurvey2::IDD )
{
	m_iAdsOpinion = adsOkay;

	//{{AFX_DATA_INIT(ChAccountDlgSurvey2)
	//}}AFX_DATA_INIT
}


ChAccountDlgSurvey2::~ChAccountDlgSurvey2()
{
}



void ChAccountDlgSurvey2::GetWizardPageData( ChString& strData )
{
	static char*	pstrAdOpinion[] = {	"no problem",
										"will pay",
										"won't pay" };

 	UpdateNameValue( strData, "ads_opinion", pstrAdOpinion[m_iAdsOpinion]  );
}



void ChAccountDlgSurvey2::DoDataExchange(CDataExchange* pDX)
{
	ChWizardPage::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(ChAccountDlgSurvey2)
	DDX_Radio(pDX, IDC_RADIO_SURVEY_GOFORIT, m_iAdsOpinion);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( ChAccountDlgSurvey2, ChWizardPage )
	//{{AFX_MSG_MAP(ChAccountDlgSurvey2)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChAccountDlgSurvey2 message handlers
----------------------------------------------------------------------------*/

// $Log$
// Revision 1.1.1.1  2003/02/03 18:52:22  uecasm
// Import of source tree as at version 2.53 release.
//
