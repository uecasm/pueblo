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

	Interface for the ChTextOutput class.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHTEXTOUTPUT_H )
#define _CHTEXTOUTPUT_H

#include <ChHtpCon.h>
#include <ChHtmWnd.h>
#include <ChHtmlSettings.h>
#include <ChPane.h>


/*----------------------------------------------------------------------------
	ChTextOutputWnd class
----------------------------------------------------------------------------*/

class ChTextOutputWnd : public ChHtmlWnd, public ChPaneWndMethods
{
	public:
		ChTextOutputWnd( ChWorldMainInfo* pInfo, ChTextOutput* pTextOutput );
		virtual ~ChTextOutputWnd();

		virtual void GetIdealSize( ChSize& size );

		#if defined( CH_MSW )

		inline bool IsValid() { return (0 != m_hWnd); }

		#endif	// defined( CH_MSW )

		inline ChWorldMainInfo* GetMainInfo() { return m_pMainInfo; }
		inline ChTextOutput* GetTextOutput() { return m_pTextOutput; }

		virtual void OnMouseUp();

	protected:
											/* ClassWizard generated virtual
												function overrides */
		//{{AFX_VIRTUAL(ChTextOutputWnd)
		virtual BOOL OnCommand( WPARAM wParam, LPARAM lParam );
		//}}AFX_VIRTUAL

	protected:
		virtual void OnHotSpot(  chparam userData, const ChString& strDocURL );
		virtual bool OnSelectHotSpot( int x, int y, ChPoint& ptRel,
										chparam userData,
										const ChString& strDocURL );

											// Image tag callback method

		virtual bool OnInline( const char* pstrArgs, const ChString& strDocURL );

		virtual bool OnRedirect( const ChString& strURL, bool boolWebTracker );    

										/* Handle form submit */
		virtual void OnSubmitForm( const ChString& strCommand, const ChString& strMD5,
												const ChString& strFormData );

		// called when a requested URL has been sucessfully loaded
		virtual void OnLoadComplete( const ChString& strFile, const ChString& strURL, 
											const ChString& strMimeType,
											chparam userData );
		// called when load error occurs for user requested URLs only.
		virtual void OnLoadError( chint32 lError, const ChString& strErrMsg, 
								const ChString& strURL,	chparam userData );

		virtual void OnRightMouseDown( const CPoint& point,
										const ChString& strWindowName )
				{
					DoPopupMenu( point, strWindowName );
				}

	public:
		virtual void OnTrace( const ChString& strMsg, int iType );

	protected:
		inline const ChString& GetMenuWindow() { return m_strMenuWindow; }

		void DoPopupMenu( CPoint point, const ChString& strWindowName );

	private:
		ChWorldMainInfo*	m_pMainInfo;
		ChTextOutput*		m_pTextOutput;

		ChString				m_strMenuWindow;

	#if !defined( CH_UNIX )

	protected:
		//{{AFX_MSG(ChTextOutputWnd)
		afx_msg void OnKillFocus(CWnd* pNewWnd);
		afx_msg void OnSetFocus(CWnd* pOldWnd);
		afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
#if defined( CH_PUEBLO_PLUGIN )
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
#endif
	//}}AFX_MSG
		afx_msg LONG OnHTTPLoadComplete(UINT wParam, LONG lParam );
		afx_msg LONG OnExecuteScript(UINT wParam, LONG lParam );

		DECLARE_MESSAGE_MAP()

	#endif	// !defined( CH_UNIX )
};


/*----------------------------------------------------------------------------
	ChTextOutput class
----------------------------------------------------------------------------*/

class ChTextOutput
{
	friend class ChRawDataWnd;

	public:
		enum tagConstants { idealCharWidth = 80, minCharWidth = 20,
							idealCharHeight = 25, minCharHeight = 1,
							charsPerLine = 80 };

	public:
		ChTextOutput( ChWorldMainInfo* pMainInfo );
		~ChTextOutput();

		inline bool IsShown() { return m_boolShown; }
		inline bool IsLogging() { return m_boolLogging; }

		inline void RemapColor( chuint32 luOldColor, chuint32 luNewColor )
		{
			GetOutputWnd()->RemapColors( 1, &luOldColor, &luNewColor );
		}

		void Show( bool boolShow = true );
		void SetFocus();
		void Clear();
		void Reset();

		inline void SetAutoScroll( bool boolAutoScroll = true )
				{
					GetOutputWnd()->ShowAppend( boolAutoScroll );
				}

		inline void SetBufferLimit( chint32 lLimit = 0xffff )
				{
					GetOutputWnd()->SetBufferLimit( lLimit );
				}

		inline chuint32 GetDefForeColor()
				{
					ChHtmlSettings*		pSettings;

					pSettings = GetOutputWnd()->GetSettings();

					return pSettings->GetTextColor();
				}

		void Add( const ChString& strText, bool boolOutputToDebug = true );

		void LoadFile( const ChString& strFilename, const ChString& strURL,
						const ChString& strHTML, const ChString& strMimeType );

		void UpdatePreferences();

		void ToggleLogging();
		bool SetLogging( const ChString& strFilePath, chflag32 flOptions = 0 );
		
#if !defined( CH_PUEBLO_PLUGIN )
		bool CheckEditMenuItem( EditMenuItem item );
		void DoEditMenuItem( EditMenuItem item );
#endif

		bool DoPaneCommand( ChPane::ChPaneCmd paneCmd, const char* pstrArgs );

		void LoadTextOutURL( const ChString& strURL )
				{
					m_pOutWnd->LoadURL( strURL );
				}

		inline ChTextOutputWnd* GetOutputWnd() { return m_pOutWnd; }

	protected:
		inline ChWorldMainInfo* GetMainInfo() { return m_pMainInfo; }

		void DoPaneOpen( const ChString& strFilename, const ChString& strMimeType,
							const ChString& strURL, const ChString& strHTML );
		void DoPaneClose( const ChString& strHTML );

		void CreateOutputWindow();

		bool GetLogFilePath( ChString& strFilePath, chflag32* pflOptions );

	protected:
		ChWorldMainInfo*		m_pMainInfo;

		ChTextOutputWnd*		m_pOutWnd;

		static chint16			m_sMinWidth;
		static chint16			m_sIdealWidth;
		static chint16			m_sMinHeight;
		static chint16			m_sIdealHeight;

		bool					m_boolLogging;
		ChString					m_strFilename;

		bool					m_boolShown;
};


#endif	// !defined( _CHTEXTOUTPUT_H )

// $Log$
