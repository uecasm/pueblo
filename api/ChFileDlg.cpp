/*----------------------------------------------------------------------------
  _   _ _  _                 ____        _                     _
 | | | | || |               |  __|      | |                   (_)
 | | | | || |_ _ __  __ _   | |_  _ __ _| |_ ___ _ ___ __ _ __ _ ___  ___ ___
 | | | | |_  _| '__|/ _` |  |  _|| '_ \_   _| _ \ '__|'_ \ '__| | __|/ _ \ __|
 | '-' | || | | |  | (_| |  | |__| | | || ||  __/ | | |_) ||  | |__ |  __/__ |
  \___/|_||_| |_|   \__,_|  |____|_| |_||_| \___|_| | .__/_|  |_|___|\___|___|
                                                    | |     
                                                    |_|

    The contents of this file are subject to the Andromedia Public
	License Version 1.0 (the "License"); you may not use this file
	except in compliance with the License. You may obtain a copy of
	the License at http://pueblo.sf.net/APL/

    Software distributed under the License is distributed on an
	"AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
	implied. See the License for the specific language governing
	rights and limitations under the License.

    The Original Code is Pueblo/UE client code, first released April 1, 2002.

    The Initial Developer of the Original Code is Ultra Enterprises.
	Portions created by Andromedia are Copyright (C) 1998 Andromedia
	Incorporated.  All Rights Reserved.

	Andromedia Incorporated                         415.365.6700
	818 Mission Street - 2nd Floor                  415.365.6701 fax
	San Francisco, CA 94103

    Contributor(s):
	--------------------------------------------------------------------------
	   Ultra Enterprises:   Gavin Lambert

					Wrote and designed this class; it was not originally part of
					the Pueblo client.

------------------------------------------------------------------------------

	This file contains the implementation of the ChFileDialog dialog class,
	which is a slight variation of CFileDialog.  Among other things, it
	fixes up a problem with occlusion by the sizing grip.

----------------------------------------------------------------------------*/

#include "headers.h"

#include <cderr.h>

#include <ChUtil.h>
#include <ChFileDlg.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	ChFileDialog class
----------------------------------------------------------------------------*/

ChFileDialog::ChFileDialog( BOOL bLoad,
														LPCTSTR strDefExt,
														LPCTSTR strFilename,
														DWORD flags,
														LPCTSTR strFilter,
														CWnd* pParent ) :
		CFileDialog( bLoad, strDefExt, strFilename, flags, strFilter, pParent )
{
	chuint32	flSysProps = ChUtil::GetSystemProperties();

	// BFC by default uses the Windows 2000 structure size for OPENFILENAME;
	// we need to alter that so that it's compatible with Windows 9x.
#if (_WIN32_WINNT >= 0x0500)
	m_ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
#else
	// OPENFILENAME_SIZE_VERSION_400 isn't defined, but the OPENFILENAME structure should be the
	// correct size now...
	m_ofn.lStructSize = sizeof(OPENFILENAME);
#endif

	if (flSysProps & CH_PROP_WIN95) {
		m_ofn.Flags |= OFN_EXPLORER;
	}

	if (flSysProps & CH_PROP_LONG_FILENAMES) {
		m_ofn.Flags |= OFN_LONGNAMES;
	}
}

void ChFileDialog::SetTemplate(HMODULE hModule, LPCTSTR nWin3ID, LPCTSTR nWin4ID) {
	m_ofn.hInstance = (hModule) ? hModule : AfxGetInstanceHandle();
	CFileDialog::SetTemplate(nWin3ID, nWin4ID);
}

int ChFileDialog::DoModal() 
{
	int		iResult;

	m_iModalResult = 0;

	iResult = CFileDialog::DoModal();

	if (IDCANCEL == iResult) {
		if (m_iModalResult) {
			// Alternate modal result (presumably from a user button, but we get it
			// from a call to EndDialog instead).
			iResult = m_iModalResult;
		}
#if defined( CH_DEBUG )
		else {
			DWORD	dwExtendedError;
	
			dwExtendedError = CommDlgExtendedError();
			if (dwExtendedError) {
				switch (dwExtendedError) {
					case CDERR_NOHINSTANCE:
						TRACE0( "The ENABLETEMPLATE flag was specified in the "
										"Flags member of a structure for the "
										"corresponding common dialog box, but "
										"the application failed to provide a "
										"corresponding instance handle." );
						break;
	
					case CDERR_DIALOGFAILURE:
						TRACE0( "The common dialog box procedure's call to "
										"the DialogBox function failed. For "
										"example, this error occurs if the common "
										"dialog box call specifies an invalid window "
										"handle.\n" );
						break;
	
					case CDERR_FINDRESFAILURE:
						TRACE0( "The common dialog box procedure failed to "
										"find a specified resource.\n" );
						break;
	
					case CDERR_INITIALIZATION:
						TRACE0( "The common dialog box procedure failed "
										"during initialization. This error often "
										"occurs when insufficient memory is "
										"available.\n" );
						break;
	
					case CDERR_LOADRESFAILURE:
						TRACE0( "The common dialog box procedure failed "
										"to load a specified resource.\n" );
						break;
	
					case CDERR_LOCKRESFAILURE:
						TRACE0( "The common dialog box procedure failed "
										"to lock a specified resource.\n" );
						break;
	
					default:
						TRACE1( "Error %lu in ChFileDialog::DoModal()\n", dwExtendedError );
						break;
				}
			}
		}
#endif	// defined( CH_DEBUG )
	}

	return iResult;
}

void ChFileDialog::OnInitDone() {
	CFileDialog::OnInitDone();
	
	// We need to change the z-order a bit - specifically, move the sizing grip
	// behind the custom template.  Otherwise, it occludes any buttons on the
	// right-hand side (below the Cancel button), which looks really ugly.
	CWnd *hGrip = CWnd::FromHandle(::FindWindowEx(GetParent()->GetSafeHwnd(),
						0, "ScrollBar", ""));
	if(hGrip && hGrip->GetStyle() & SBS_SIZEGRIP) {
		// Yes, it's definitely the sizing grip :)
		// Now find the custom box control...
		CWnd *hCustom = CWnd::FromHandle(::FindWindowEx(GetParent()->GetSafeHwnd(),
						hGrip->GetSafeHwnd(), MAKEINTATOM(32770), 0));
		// Note: I'm not sure whether that atom number is standard or not; it always
		// comes up that way on my system, but it could be different on other systems.
		// In any case, if it isn't what I'm expecting, the whole adjustment will fail
		// and the size grip will occlude other buttons.  It's hardly critical.
		if(hCustom) {
			// Found it.  Now we can move the grip in behind:
			hGrip->SetWindowPos(hCustom, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |
							SWP_NOACTIVATE);
			// But wait, there's more!  Leaving it there would make the grip
			// invisible, being hidden behind the custom dialog.  In order to get
			// it back again, we need to make the custom dialog transparent:
			hCustom->ModifyStyleEx(0, WS_EX_TRANSPARENT);
			// And that should do the trick! :)
		}
	}
	// Finally, the primary button in any custom template we use mistakenly gets
	// given the default style.  This will get rid of it.
	HWND hwnd = 0;
	while((hwnd = ::FindWindowEx(GetSafeHwnd(), hwnd, "Button", 0)) != NULL) {
		// Found a button, check its style
		CWnd *hBtn = CWnd::FromHandle(hwnd);
		if((hBtn->GetStyle() & 0xF) == BS_DEFPUSHBUTTON) {
			// Seems to be a default styled button
			hBtn->ModifyStyle(BS_DEFPUSHBUTTON, BS_PUSHBUTTON);
		}
	}
}

BEGIN_MESSAGE_MAP( ChFileDialog, CFileDialog )
END_MESSAGE_MAP()
