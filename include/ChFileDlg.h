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

	This file contains the interface for the ChFileDialog dialog class,
	which is a slight variation of CFileDialog.  Among other things, it
	fixes up a problem with occlusion by the sizing grip.

----------------------------------------------------------------------------*/

#if !defined( CH_FILEDLG_H_ )
#define CH_FILEDLG_H_

/*----------------------------------------------------------------------------
	ChFileDialog class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChFileDialog : public CFileDialog {
	public:
		ChFileDialog( BOOL bLoad,
									LPCTSTR strDefExt = NULL,
									LPCTSTR strFilename = NULL,
									DWORD flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
									LPCTSTR strFilter = NULL,
									CWnd* pParent = 0);

		// Note: you should call SetTemplate ASAP; while in theory this should
		// work without using a custom template, this has not been tested (since
		// all of the Pueblo dialogs use one), so correct behaviour is not
		// guaranteed.
		void SetTemplate(HMODULE hModule, LPCTSTR nWin3ID, LPCTSTR nWin4ID);
		void SetTemplate(HMODULE hModule, UINT nWin3ID, UINT nWin4ID) {
			SetTemplate(hModule, MAKEINTRESOURCE(nWin3ID), MAKEINTRESOURCE(nWin4ID));
		}

		virtual int DoModal();
		
		void EndDialog(int nResult) {
			// A standard file dialog cannot return a value directly.  Calling
			// this will set the m_iModalResult member.  This value will be
			// returned from the DoModal() call.
			//
			// NOTE: Use only '0' for error or a control ID; calling EndDialog(1)
			// for example will make your application think the user clicked the
			// OK button, leading to erroneous behaviour.
			m_iModalResult = nResult;
			GetParent()->PostMessage(WM_COMMAND, IDCANCEL);
		}

	protected:
		virtual void OnInitDone();
		
		int m_iModalResult;

	DECLARE_MESSAGE_MAP()
};

#endif // CH_FILEDLG_H_
