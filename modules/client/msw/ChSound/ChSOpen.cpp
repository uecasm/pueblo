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

	Implementation of the ChSoundOpenFileDlg, based on the Windows common
	dialog.  This dialog manages opening sound files.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <dlgs.h>

#include "ChSound.h"
#include "ChSOpen.h"
#include "ChSoundInfo.h"

#include <cderr.h>
#include "MemDebug.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define OPEN_DLG_FLAGS		(OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST |\
								OFN_HIDEREADONLY)
#define OPEN_DLG_FILTER		"Wave Files (*.wav)|*.wav|"\
								"All Files (*.*)|*.*||"


/*----------------------------------------------------------------------------
	ChSoundOpenFileDlg class
----------------------------------------------------------------------------*/

ChSoundOpenFileDlg::ChSoundOpenFileDlg( ChSoundMainInfo* pMainInfo,
										chuint16 suVolume,
										const ChString& strAlertSoundPath,
										const ChString& strAlertSoundTitle,
										CWnd* pParent ) :
		ChFileDialog( true, "wav", strAlertSoundTitle, OPEN_DLG_FLAGS,
						OPEN_DLG_FILTER ),
		m_pMainInfo( pMainInfo ),
		m_suVolume( suVolume ),
		m_strInitialTitle( strAlertSoundTitle ),
		m_strInitialPath( strAlertSoundPath ),
		m_boolNoSound( false )
{
	m_flSysProps = ChUtil::GetSystemProperties();

	if (m_suVolume < 0x7FFF)
	{										/* Make sure they can hear tested
												sounds */
		m_suVolume = 0x7FFF;
	}

	m_ofn.lpstrInitialDir = m_strInitialPath;

	SetTemplate(
#if defined( CH_PUEBLO_PLUGIN )
			0,		// defaults to AfxGetInstanceHandle()
#else
			ChSoundDLL.hModule,
#endif
			IDD_SOUND_FILE_OPEN, IDD_SOUND_FILE_OPEN_95);

	//{{AFX_DATA_INIT(ChSoundOpenFileDlg)
	//}}AFX_DATA_INIT
}


int ChSoundOpenFileDlg::DoModal() 
{
	int			iResult;

	iResult = ChFileDialog::DoModal();

	if (m_boolNoSound)
	{
		m_szFileTitle[0] = 0;
		m_szFileName[0] = 0;
	}

	return iResult;
}


void ChSoundOpenFileDlg::DoDataExchange( CDataExchange* pDX )
{
	ChFileDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ChSoundOpenFileDlg)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( ChSoundOpenFileDlg, ChFileDialog )
	//{{AFX_MSG_MAP(ChSoundOpenFileDlg)
	ON_BN_CLICKED(IDC_NO_SOUND, OnNoSound)
	ON_BN_CLICKED(IDC_TEST, OnTest)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChSoundOpenFileDlg message handlers
----------------------------------------------------------------------------*/

void ChSoundOpenFileDlg::OnNoSound() 
{
	m_boolNoSound = true;
	EndDialog( IDOK );
}


void ChSoundOpenFileDlg::OnTest() 
{
	ChString	strCurrent;

	if (m_flSysProps & CH_PROP_WIN95)
	{
		//char	strBuffer[_MAX_PATH];

		//SendMessage( CDM_GETFILEPATH, sizeof( strBuffer ),
		//				(LPARAM)(char*)strBuffer );
		//strCurrent = strBuffer;
		strCurrent = GetPathName();
	}
	else
	{
		CEdit*	pFilenameEdit;

		if (pFilenameEdit = (CEdit*)GetDlgItem( 1152 ))
		{
			pFilenameEdit->GetWindowText( strCurrent );
		}
	}

#ifdef _DEBUG
	afxDump << "Testing sound: \"" << strCurrent << "\".\n";
#endif

	if (!strCurrent.IsEmpty() && GetMainInfo())
	{
		ChSoundInfo		info( false, true, m_suVolume );

		info.SetFilename( strCurrent );
		info.SetDeviceType( devWave );

		if (!GetMainInfo()->DoPlay( &info ))
		{
			AfxMessageBox( IDS_INVALID_SOUND );
		}
	}
}

// $Log$
// Revision 1.1.1.1  2003/02/03 18:52:59  uecasm
// Import of source tree as at version 2.53 release.
//
