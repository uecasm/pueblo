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

	Implementation of the ChLogSaveAsDlg, based on the Windows common
	dialog.  This dialog manages saving log files.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <dlgs.h>

#include "ChSound.h"
#include "ChSaveAs.h"

#include <cderr.h>

#if defined( _DEBUG )
	#undef THIS_FILE
	static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define SAVE_DLG_FLAGS			(OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY)
#define SAVE_DLG_FILTER			"Text Files (*.txt)|*.txt|"\
								"HTML Files (*.htm)|*.htm|"\
								"All Files (*.*)|*.*||"
#define SAVE_DLG_FILTER_LONG	"Text Files (*.txt)|*.txt|"\
									"HTML Files (*.htm;*.html)|*.htm;*.html|"\
									"All Files (*.*)|*.*||"


/*----------------------------------------------------------------------------
	ChLogSaveAsDlg class
----------------------------------------------------------------------------*/

ChLogSaveAsDlg::ChLogSaveAsDlg( CWnd* pParent ) :
		ChFileDialog( false, "txt", 0, SAVE_DLG_FLAGS ),
		m_boolHTML( false ),
		m_boolEntireBuffer( false )
		//,m_iFilterSel( 0 )
{
	DWORD		dwDlgFlags = SAVE_DLG_FLAGS;
	char*		pBuffer;

	m_flSysProps = ChUtil::GetSystemProperties();

											/* Get the default directory for
												log files */
	ChUtil::GetAppDirectory( m_strDefDir );
	m_strDefDir += SAVE_LOG_DIR;
	m_strDefDir += "\\";
											/* Make sure the directory tree
												exists */
	ChUtil::CreateDirectoryTree( m_strDefDir );

	if (m_flSysProps & CH_PROP_LONG_FILENAMES)
	{
		m_strFilter = SAVE_DLG_FILTER_LONG;

		dwDlgFlags |= OFN_LONGNAMES;
	}
	else
	{
		m_strFilter = SAVE_DLG_FILTER;
	}

	SetTemplate(
#if defined( CH_PUEBLO_PLUGIN )
			0,		// defaults to AfxGetInstanceHandle()
#else
			ChWorldDLL.hModule,
#endif
			IDD_LOG_FILE_SAVE_AS, IDD_LOG_FILE_SAVE_AS_95);

	m_ofn.Flags |= dwDlgFlags;
	m_ofn.lpstrInitialDir = m_strDefDir;
											/* Translate filter into commdlg
												format (lots of \0's) */

	pBuffer = m_strFilter.GetBuffer(0);		// Modify the buffer in place
	while ((pBuffer = _tcschr( pBuffer, '|' )) != 0)
	{
		*pBuffer++ = '\0';
	}
	m_ofn.lpstrFilter = m_strFilter;

	//{{AFX_DATA_INIT(ChLogSaveAsDlg)
	//}}AFX_DATA_INIT
}


void ChLogSaveAsDlg::DoDataExchange( CDataExchange* pDX )
{
	ChFileDialog::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(ChLogSaveAsDlg)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( ChLogSaveAsDlg, ChFileDialog )
	//{{AFX_MSG_MAP(ChLogSaveAsDlg)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChLogSaveAsDlg message handlers
----------------------------------------------------------------------------*/

BOOL ChLogSaveAsDlg::OnInitDialog() 
{
	return ChFileDialog::OnInitDialog();
}


int ChLogSaveAsDlg::DoModal()
{
	int			iResult;

	iResult = ChFileDialog::DoModal();

#ifdef _DEBUG
	afxDump << "ChLogSaveAsDlg: result == " << iResult << ", HTML flag == "
					<< (BOOL)IsHTML() << ", filename == \"" << GetPathName() << "\"\n";
#endif

/*
	if (IDOK == iResult)
	{
		m_strFileExtension = ChFileDialog::GetFileExt();
		TRACE1("ChLogSaveAsDlg - extension == \"%s\"\n", (LPCSTR)m_strFileExtension);
		if (m_strFileExtension.IsEmpty())
		{
			TRACE1("Choosing extension - filterSel == %d\n", m_iFilterSel);
			switch( m_iFilterSel )
			{
				case 0:						// Text Files (*.txt)
				{
					m_strFileExtension = "txt";
					break;
				}

				case 1:						// HTML Files (*.htm;*.html)
				{
					if (m_flSysProps & CH_PROP_LONG_FILENAMES)
					{
						m_strFileExtension = "html";
					}
					else
					{
						m_strFileExtension = "htm";
					}
					break;
				}

				default:					// Everything else
				{
					m_strFileExtension = "txt";
					break;
				}
			}

			m_strFileExtension.MakeLower();
											*/ /* Append the extension to the
												file name.  (Note that
												ChFileDialog stores the name
												twice... with and without
												path... so we need to append
												it twice.) */ /*
			strcat( m_szFileTitle, "." );
			strcat( m_szFileTitle, m_strFileExtension );
			strcat( m_szFileName, "." );
			strcat( m_szFileName, m_strFileExtension );
		}
		else
		{
			m_strFileExtension.MakeLower();
		}

		ASSERT( !m_strFileExtension.IsEmpty() );

		if ((m_strFileExtension == "htm") || (m_strFileExtension == "html"))
		{
			m_boolHTML = true;
		}
	}
*/

	return iResult;
}

void ChLogSaveAsDlg::OnTypeChange() {
	ChFileDialog::OnTypeChange();
	
	// Set the default file extension based on the new filter.
	switch(m_ofn.nFilterIndex) {
		case 2:			// HTML Files (*.htm;*.html)
			SetDefExt("html");
			m_boolHTML = true;
			break;
		case 1:			// Text Files (*.txt)
		default:
			SetDefExt("txt");
			m_boolHTML = false;
			break;
	}
}

/*----------------------------------------------------------------------------
	ChLogSaveAsDlg message handlers
----------------------------------------------------------------------------*/

void ChLogSaveAsDlg::OnDestroy() 
{
	ChFileDialog::OnDestroy();
											/* Set variables before the window
												goes away */
	if (m_flSysProps & CH_PROP_WIN95)
	{
//		CComboBox*	pFilterCombo;
//		CWnd*		pDlgAddition;
		CButton*	pBtn;

		/*
		if (pFilterCombo = (CComboBox*)GetDlgItem( cmb1 ))
		{
			m_iFilterSel = pFilterCombo->GetCurSel();
		}
		*/
											/* Under Windows 95, the additions
												to the standard dialog are in
												a child dialog of their own.
												This child dialog is the last
												dialog in the list. */

		/*
		if (pDlgAddition = GetWindow( GW_CHILD ))
		{
			pDlgAddition = pDlgAddition->GetWindow( GW_HWNDLAST );
		}

		pBtn = (CButton*)pDlgAddition->GetDlgItem( IDC_CHECK_ENTIRE_BUFFER );
		*/
		pBtn = (CButton*)GetDlgItem( IDC_CHECK_ENTIRE_BUFFER );
		if (pBtn)
		{
			m_boolEntireBuffer = (pBtn->GetCheck() != 0);
		}
	}
	else
	{
		CComboBox*	pFilterCombo;
		CButton*	pBtn;

		if (pFilterCombo = (CComboBox*)GetDlgItem( 1136 ))
		{
			//m_iFilterSel = pFilterCombo->GetCurSel();
			//** NOTE: this will seriously break Win3.1 support, it will need to be
			//   fixed if you want to support Win3.1 (or Win32s) again.
			::MessageBox(0, "Functionality impaired in ChSaveAs.\nNotify UE if you get "
									"this message.", "Version incompatibility", MB_OK | MB_ICONSTOP);
		}

		if (pBtn = (CButton*)GetDlgItem( IDC_CHECK_ENTIRE_BUFFER ))
		{
			m_boolEntireBuffer = (pBtn->GetCheck() != 0);
		}
	}
}

// $Log$
