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

	Implementation for the ChTextOutputBar class.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <ChConst.h>
#include <ChCore.h>

#include "World.h"
#include "ChTextOutput.h"
#include "ChTextInput.h"
#include "ChWorldStream.h"
#include "ChSaveAs.h" 


#define 	PUEBLO_FORM_CMD   		TEXT( "PUEBLOFORM" )


#if defined( CH_PUEBLO_PLUGIN )
#include <AFXPRIV.H>
#include <ChSplit.h>
#endif

#include "MemDebug.h"

/*----------------------------------------------------------------------------
	Inline functions
----------------------------------------------------------------------------*/

inline bool IsTargetSelfOutput( const ChString& strTarget )
	{
		bool	boolSelf = (strTarget.CompareNoCase( "_self" ) == 0);

		return boolSelf;
	}


/*----------------------------------------------------------------------------
	class ChPaneOutWnd
----------------------------------------------------------------------------*/

class ChPaneOutWnd : public ChTextOutputWnd
{
	public:
		ChPaneOutWnd( ChWorldMainInfo* pInfo, ChTextOutput* pTextOutput,
						ChPane* pPane ) :
				ChTextOutputWnd( pInfo, pTextOutput ),
				m_pPane( pPane )
					{
					}
		virtual ~ChPaneOutWnd() {}

		virtual void OnFrameDisconnect( const ChModuleID& idNewModule );
		virtual void OnMouseUp();


	protected:
		ChPane*		m_pPane;
};


void ChPaneOutWnd::OnFrameDisconnect( const ChModuleID& idNewModule )
{
	DestroyWindow();
	delete this;
}


void ChPaneOutWnd::OnMouseUp()
{
	if (m_pPane && m_pPane->IsInternal())
	{										/* For panes within the main frame,
												we want to call the default
												OnMouseUp() method, which
												switches focus back to the
												input window */
		ChTextOutputWnd::OnMouseUp();
	}
}



/*----------------------------------------------------------------------------
	ChTextOutput class
----------------------------------------------------------------------------*/

chint16		ChTextOutput::m_sMinWidth;
chint16		ChTextOutput::m_sIdealWidth;
chint16		ChTextOutput::m_sMinHeight;
chint16		ChTextOutput::m_sIdealHeight;

ChTextOutput::ChTextOutput( ChWorldMainInfo* pMainInfo ) :
				m_pMainInfo( pMainInfo ),
				m_pOutWnd( 0 ),
				m_boolLogging( false ),
				m_boolShown( false )
{
	ChPane*		pPane;

	m_pOutWnd = new ChTextOutputWnd( GetMainInfo(), this );
	ASSERT( m_pOutWnd );

	ChPaneMgr *pPaneMgr = GetMainInfo()->GetPaneMgr();
	m_pOutWnd->SetPaneMgr( pPaneMgr );

	pPane = pPaneMgr->FindPane( ChPaneMgr::strTextName );

											// Calculate sizing values
	CClientDC	dc( pPane->GetFrameWnd() );
	TEXTMETRIC	tm;

	dc.GetTextMetrics( &tm );

	m_sMinWidth = chint16(tm.tmAveCharWidth * minCharWidth);
	m_sIdealWidth = chint16(tm.tmAveCharWidth * idealCharWidth);
	m_sMinHeight = chint16((tm.tmHeight + tm.tmExternalLeading) * minCharHeight);
	m_sIdealHeight = chint16((tm.tmHeight + tm.tmExternalLeading) * idealCharHeight);

	UpdatePreferences();
	CreateOutputWindow();
}


ChTextOutput::~ChTextOutput()
{
	ChModuleID		idModule = GetMainInfo()->GetModuleID();

	if (IsShown())
	{
		Show( false );
	}
											// Clear all panes

	GetMainInfo()->GetPaneMgr()->ShowAllPanes( idModule, false );
	GetMainInfo()->GetPaneMgr()->DestroyAllPanes( idModule );

											// Destroy the window
	if (GetOutputWnd())
	{
		GetOutputWnd()->DestroyWindow();

		delete m_pOutWnd;
		m_pOutWnd = 0;
	}
}


void ChTextOutput::Show( bool boolShow )
{
	ChPane*		pPane;
											// Get the correct pane

	pPane = GetMainInfo()->GetPaneMgr()->FindPane( ChPaneMgr::strTextName );

	if (boolShow && !IsShown())
	{										// Show the window
		pPane->Show();
		m_boolShown = true;
	}
	else if (!boolShow && IsShown())
	{										// Hide the window
		pPane->Show( false );
		m_boolShown = false;
	}
}


void ChTextOutput::SetFocus()
{
	GetOutputWnd()->SetFocus();
}


void ChTextOutput::Clear()
{
	GetOutputWnd()->NewPage();
}


void ChTextOutput::Reset()
{
	ChWorldMainInfo*	pInfo = GetMainInfo();

	ASSERT( pInfo );
	pInfo->GetPaneMgr()->DestroyAllPanes( pInfo->GetModuleID(), false );
}


void ChTextOutput::Add( const ChString& strText, bool boolOutputToDebug )
{
	if (boolOutputToDebug && GetMainInfo()->IsConnected())
	{
		GetMainInfo()->GetCore()->Trace( strText, ChCore::traceHTML );
	}

	GetOutputWnd()->AppendText( strText );
}

void ChTextOutput::LoadFile( const ChString& strFilename, const ChString& strURL,
								const ChString& strHTML,
								const ChString& strMimeType )
{
	bool					boolPaneAttrs;
	ChPane::ChPaneCmd		paneCmd;
	ChString					strPaneName;
	ChString					strPaneTitle;

	if (boolPaneAttrs = !strHTML.IsEmpty())
	{
		boolPaneAttrs = ChPane::GetPaneCmdAttr( strHTML, paneCmd );
	}

	if (boolPaneAttrs)
	{
		ASSERT( ChPane::paneOpen == paneCmd );

		DoPaneOpen( strFilename, strMimeType, strURL, strHTML );
	}
}


void ChTextOutput::ToggleLogging()
{
	if (IsLogging())
	{										// Stop logging
		GetOutputWnd()->CloseFile();
		m_boolLogging = false;
	}
	else
	{										// Get logging file path
		bool		boolContinue = false;
		ChString		strFilePath;
		chflag32	flOptions;

		#if defined( CH_MSW )
		{
			boolContinue = GetLogFilePath( strFilePath, &flOptions );
		}
		#endif	// defined( CH_MSW )

		if (boolContinue)
		{									// Turn on logging
			TRACE2("Setting logging: %s, %d.\n", (LPCSTR)strFilePath, flOptions);
			SetLogging( strFilePath, flOptions );
		}
	}
}


bool ChTextOutput::SetLogging( const ChString& strFilepath, chflag32 flOptions )
{
	ChString		strWorkingFilepath;

	if (IsLogging())
	{										// Stop logging
		GetOutputWnd()->CloseFile();
	}

	if (-1 == strFilepath.Find( '\\' ))
	{
		ChString		strDefDir;
											/* Get the default directory for
												log files */

		ChUtil::GetAppDirectory( strWorkingFilepath );
		strWorkingFilepath += SAVE_LOG_DIR;
		strWorkingFilepath += "\\";
		strWorkingFilepath += strFilepath;
	}
	else
	{
		strWorkingFilepath = strFilepath;
	}
											// Open the new file

	m_boolLogging = GetOutputWnd()->WriteFile( strWorkingFilepath, flOptions );

	return m_boolLogging;
}


void ChTextOutput::UpdatePreferences()
{
}


#if !defined( CH_PUEBLO_PLUGIN )
bool ChTextOutput::CheckEditMenuItem( EditMenuItem item )
{
	bool	boolEnable;

	switch( item )
	{
		case editMenuCopy:
		{
			chint32 lStart, lEnd;

			GetOutputWnd()->GetSel( lStart, lEnd );

			boolEnable = lStart != lEnd;
			break;
		}

		case editMenuCut:
		case editMenuPaste:
		{
			boolEnable = false;
			break;
		}

		default:
		{
			boolEnable = false;
			break;
		}
	}

	return boolEnable;
}


void ChTextOutput::DoEditMenuItem( EditMenuItem item )
{
	switch( item )
	{
		case editMenuCopy:
		{
			GetOutputWnd()->CopyToClipboard();
			break;
		}
		case editMenuCut:
		case editMenuPaste:
		default:
		{
			break;
		}
	}
}

#endif // #if !defined( CH_PUEBLO_PLUGIN )

bool ChTextOutput::DoPaneCommand( ChPane::ChPaneCmd paneCmd,
									const char* pstrArgs )
{
	ChString		strWarning;

	LOADSTRING( IDS_WARNING_OLD_PANE_FORMAT, strWarning );
	GetMainInfo()->GetCore()->Trace( strWarning, ChCore::traceWarnings );

	switch( paneCmd )
	{
		case ChPane::paneOpen:
		{
			ChString		strURL;

			ChHtmlWnd::GetHTMLHref( pstrArgs, true, strURL );

			if (!strURL.IsEmpty())
			{ 
				ChWorldFileHTTPReq*		pLoadInfo;

				pLoadInfo = new ChWorldFileHTTPReq( GetMainInfo()->GetConnectID(), 
												GetOutputWnd()->GetSafeHwnd(),
												pstrArgs );

				GetMainInfo()->GetCore()->GetURL( strURL, 0, 
								GetMainInfo()->GetStream(), 
								(chparam)pLoadInfo  );
			}
			else
			{
				TRACE1( "Empty URL passed into "
						"ChTextOutput::DoPaneCommand : %s ", pstrArgs );
			}
			break;
		}

		case ChPane::paneClose:
		{
			DoPaneClose( pstrArgs );
			break;
		}

		default:
		{
			break;
		}
	}

	return true;
}


/*----------------------------------------------------------------------------
	ChTextOutput protected methods
----------------------------------------------------------------------------*/

void ChTextOutput::DoPaneOpen( const ChString& strFilename,
								const ChString& strMimeType,
								const ChString& strURL, const ChString& strHTML )
{
	bool				boolPaneAttrs;
	ChPaneMgr*			pPaneManager = GetMainInfo()->GetPaneMgr();
	ChPane*				pPane;
	ChPaneOutWnd*		pChild;
	ChRect				rtChild( 10, 10, 300, 500 );
	ChPane::ChPaneCmd	paneCmd;
	ChString				strPaneName;
	ChString				strPaneTitle;
	chflag32			flPaneOptions;
	chint16				sIdealWidth = m_sIdealWidth;
	chint16				sIdealHeight = m_sIdealHeight;
	chint16				sMinWidth = m_sMinWidth;
	chint16				sMinHeight = m_sMinHeight;
	bool				boolCreated = true;

#ifdef _DEBUG
	::AfxTrace("ChTextOutput::DoPaneOpen(\"%s\", \"%s\", \"%s\", \"%s\")\n",
			 (LPCSTR)strFilename, (LPCSTR)strMimeType, (LPCSTR)strURL, (LPCSTR)strHTML);
#endif
	
	boolPaneAttrs = ChPane::GetPaneAttrs( strHTML, paneCmd, strPaneName,
											strPaneTitle, flPaneOptions );
	ASSERT( boolPaneAttrs );
	ASSERT( ChPane::paneOpen == paneCmd );

	ChPane::GetPaneSizeAttrs( strHTML, sIdealWidth, sIdealHeight, sMinWidth,
								sMinHeight );

	{
		char buffer[128];
		wsprintf(buffer, "ChTextOutput::DoPaneOpen(\"%s\", \"%s\", \"%s\", \"%s\")",
						(LPCSTR)strFilename, (LPCSTR)strMimeType, (LPCSTR)strURL, (LPCSTR)strHTML);
		::MessageBox(NULL, buffer, "Debug", MB_OK | MB_ICONINFORMATION);
	}

	if (!(pPane = pPaneManager->FindPane( strPaneName )))
	{
		pPane = pPaneManager->CreatePane( strPaneName, 0, sIdealWidth,
											sIdealHeight, flPaneOptions );
		ASSERT( pPane );
	}

	if (pPane->GetModuleID() != GetMainInfo()->GetModuleID())
	{
		DWORD			dwPaneStyle;

		pChild = new ChPaneOutWnd( GetMainInfo(), this, pPane );
		ASSERT( pChild );
											/* Create the new child using the
												pane frame window as the
												parent */
		#if defined( CH_MSW )

		if (flPaneOptions & paneInternal)
		{									/* Leave the border off of a banner
												pane child, since it shows up
												in a banner */

			dwPaneStyle = WS_VISIBLE | WS_VSCROLL;
		}
		else
		{
			dwPaneStyle = WS_VISIBLE | WS_BORDER | WS_VSCROLL;
		}

		if (boolCreated = (pChild->Create( rtChild, pPane->GetFrameWnd(),
											dwPaneStyle ) != FALSE))
		{
			chint16		sBorderHorz = -1;
			chint16		sBorderVert = -1;
												// Take ownership of the pane

			pPane->SetOwner( GetMainInfo()->GetModuleID(), pChild, pChild );
			pPane->SetTitle( strPaneTitle );
			pPane->SetSizePrefs( sIdealWidth, sIdealHeight, sMinWidth,
									sMinHeight );

												/* Set view indents, if
													necessary */

			ChPane::GetPaneBorderAttrs( strHTML, sBorderHorz, sBorderVert );
			if ((sBorderHorz != -1) && (sBorderVert != -1))
			{
				ChRect		rtIndent;

				pChild->GetViewIndents( rtIndent );

				if (sBorderHorz != -1)
				{
					rtIndent.left = rtIndent.right = sBorderHorz;
				}

				if (sBorderVert != -1)
				{
					rtIndent.top = rtIndent.bottom = sBorderVert;
				}

				pChild->SetViewIndents( rtIndent );
			}
		}
		else

		#endif	// defined( CH_MSW )

		{
			delete pChild;
			GetMainInfo()->GetCore()->
				Trace( "ChTextOutput::DoPaneOpen : Error creating pane.",
						ChCore::traceMiscMessages );
		}
	}
	else
	{
		pChild = (ChPaneOutWnd*)pPane->GetWindow();
	}

	if (boolCreated)
	{										// Display the file

		pChild->DisplayFile( strFilename, strMimeType, ChHtmlWnd::fileReplace,
								strURL );
											/* Size the pane if necessary.  The
												pane is sized if it's either
												not sizeable, or if it hasn't
												been automatically sized to
												a previous size. */

		if (!pPane->IsSizeable() || !pPane->IsSized())
		{
			pPane->SetSize( sIdealWidth, sIdealHeight );
		}
											// Turn off scrolling if necessary
		if (pPane->IsNoScroll())
		{
			CRect	rtEdges( 0, 0, 0, 0 );

			pChild->AllowScroll( false, false );
			pChild->SetViewIndents( rtEdges );
		}
												// Finally show the pane
		pPane->Show();
	}
}


void ChTextOutput::DoPaneClose( const ChString& strHTML )
{
	bool					boolPaneAttrs;
	ChString					strPaneName;
	ChString					strPaneTitle;

	ASSERT( !strHTML.IsEmpty() );

	boolPaneAttrs = ChPane::GetPaneNameAttr( strHTML, strPaneName );

	ASSERT( boolPaneAttrs );

	if (!strPaneName.IsEmpty())
	{
		GetMainInfo()->GetPaneMgr()->DestroyPane( strPaneName );
	}
}


void ChTextOutput::CreateOutputWindow()
{
	ChRect			rtChild( -100, -100, -1, -1 );
	bool			boolCreated;
	ChPane*			pPane;
	ChString			strDummy;

	pPane = GetMainInfo()->GetPaneMgr()->FindPane( ChPaneMgr::strTextName );

											/* Create the new child using the
												pane frame window as the
												parent */

	boolCreated = (GetOutputWnd()->Create( rtChild, pPane->GetFrameWnd() ) != FALSE);
	ASSERT( boolCreated );
											// Set the child into the view

	pPane->SetOwner( GetMainInfo()->GetModuleID(), GetOutputWnd(),
						GetOutputWnd() );
	pPane->SetSizePrefs( m_sIdealWidth, m_sIdealHeight, m_sMinWidth,
							m_sMinHeight );
}


#if defined( CH_MSW )

bool ChTextOutput::GetLogFilePath( ChString& strFilePath, chflag32* pflOptions )
{
	bool			boolSuccess;
	ChLogSaveAsDlg	fileDlg;
	chflag32		flTempOptions;

	if (0 == pflOptions)
	{
		pflOptions = &flTempOptions;
	}

	ASSERT( 0 != pflOptions );

	*pflOptions = ChHtmlWnd::writeNew;

	boolSuccess = (IDOK == fileDlg.DoModal());

	if (boolSuccess)
	{										// Cool -- they pressed okay
		strFilePath = fileDlg.GetPathName();

		if (fileDlg.IsHTML())
		{
			*pflOptions |= ChHtmlWnd::writeHTML;
		}

		if (fileDlg.IsEntireBuffer())
		{
			*pflOptions |= ChHtmlWnd::writeAll;
		}
	}

	return boolSuccess;
}

#endif	// defined( CH_MSW )


/*----------------------------------------------------------------------------
	ChTextOutputWnd class
----------------------------------------------------------------------------*/

ChTextOutputWnd::ChTextOutputWnd( ChWorldMainInfo* pInfo,
									ChTextOutput* pTextOutput ) :
					#if defined( CH_PUEBLO_PLUGIN )
					 ChHtmlWnd( pInfo->GetCore()->GetHTTPConn() ),
					#endif
					m_pMainInfo( pInfo ),
					m_pTextOutput( pTextOutput )
{
	SetAnchorTarget( ChString( TEXT("_webtracker" ) ) );

	SetBufferLimit( 0x7FFF );				// 32k of text
}


ChTextOutputWnd::~ChTextOutputWnd()
{
	DestroyWindow();
}


void ChTextOutputWnd::GetIdealSize( ChSize& size )
{
	GetDocumentSize( size );
}


void ChTextOutputWnd::OnMouseUp()
{
	if (GetMainInfo()->GetTextInput()->IsShown())
	{
		chint32		lStart, lEnd;

		GetSel( lStart, lEnd );

		if (lStart == lEnd)
		{
			GetMainInfo()->GetTextInput()->SetFocus();
		}
	}
}


/*----------------------------------------------------------------------------
	ChTextOutputWnd protected methods
----------------------------------------------------------------------------*/

bool ChTextOutputWnd::OnSelectHotSpot( int x, int y, ChPoint& ptRel, chparam userData,
											const ChString& strDocURL )
{
	const char* pstrArgs = (const char*)userData;

											/* Notification on hotspot
												selection */
	if (pstrArgs)
	{
		ChPane::ChPaneCmd		paneCmd;
											/* Check to see if this is a pane
												anchor.  If it is, send it to
												the pane command handler. */

		if (ChPane::GetPaneCmdAttr( pstrArgs, paneCmd ))
		{
			GetTextOutput()->DoPaneCommand( paneCmd, pstrArgs );
		}
		else
		{									/* Otherwise send the anchor
												through the hook chain */
			bool				boolProcessed;

			#if defined( CH_DEBUG )
			{
				ChString		strDump( pstrArgs );

				if (strDump.GetLength() > 150)
				{
					strDump = strDump.Left( 150 );
					strDump += " [...]";
				}

				TRACE1( "HTML Hot spot: %s\n", (const char*)strDump );
			}
			#endif	// defined( CH_DEBUG )

			boolProcessed = GetMainInfo()->DoCommand( pstrArgs, ptRel.x,
														ptRel.y );
			if (boolProcessed)
			{								/* Set focus back to the input
												window */

				GetMainInfo()->GetTextInput()->SetFocus();
			}
			else
			{								/* The selection of a hot spot was
												not processed, check if there
												is a HREF, if it does then
												launch the Web browser if we
												have a valid URL */
			   	ChString		strValue;

				if (ChHtmlWnd::GetHTMLAttribute( pstrArgs, ATTR_HREF,
													strValue ))
				{
					ChString		strTarget;

					ChHtmlWnd::GetHTMLAttribute( pstrArgs, ATTR_TARGET,
													strTarget );
					if (IsTargetSelfOutput( strTarget ))
					{
						return ChHtmlWnd::OnSelectHotSpot( x, y, ptRel, userData,
													strDocURL );
					}
					else
					{
						ChURLParts		urlMap;

						if (urlMap.GetURLParts( strValue, strDocURL ))
						{

							OnRedirect( urlMap.GetURL(), false );

						}
					}
				}
			}
		}
	}
	return true;
}

bool ChTextOutputWnd::OnRedirect(  const ChString& strURL, bool boolWebTracker  )
{
	GetMainInfo()->GetCore()->DisplayWebPage( strURL, boolWebTracker 
												? ChCore::browserWebTracker 
												: ChCore::browserUserPref );
	return true;
}


void ChTextOutputWnd::OnSubmitForm( const ChString& strAction, const ChString& strMD5,
												const ChString& strFormData )
{

								// Data is of the form:
								//	xch_cmd="action?formdata"	 
	ChString strCommand( TEXT( "xch_cmd=" ) );
								// quote the data we pass
	strCommand += TEXT( '"' );

	if (strAction.IsEmpty() || strMD5.IsEmpty() ||
			!GetMainInfo()->VerifyMD5( strMD5 ))
	{ 										/* This is not secure, we allow
												only PUEBLO form command */

		if (strAction.IsEmpty() || lstrcmpi( strAction, PUEBLO_FORM_CMD ))
		{
			ChString strMsg;

			strMsg.Format( "Invalid form action : %s", LPCSTR(strAction) );
			OnTrace( strMsg, ChHtmlWnd::traceWarning );
		}

		strCommand += PUEBLO_FORM_CMD;
	}
	else
	{										/* MD5 matches, we will allow any
												command specified */
		strCommand += strAction;
	}

	strCommand += TEXT( " ?" );
	strCommand += strFormData;
	strCommand += TEXT( '"' );

	GetMainInfo()->DoCommand( strCommand );
}


void ChTextOutputWnd::OnLoadComplete( const ChString& strFile,
										const ChString& strURL,
										const ChString& strMimeType,
										chparam userData )
{
											/* Set the window to not scroll
												to the end, and to have no
												text limit */
	ShowAppend( false );
	SetBufferLimit( 0 );

	GetMainInfo()->SetCurrentURL( strURL );
	GetMainInfo()->SetLoadPending( false );
											// Load the file to display

	ChHtmlWnd::OnLoadComplete( strFile, strURL, strMimeType, userData );
}


// called when load error occurs for user requested URLs only.
void ChTextOutputWnd::OnLoadError( chint32 lError, const ChString& strErrMsg, 
						const ChString& strURL,	chparam userData )
{
	GetMainInfo()->SetLoadPending( false );
	ChString		strErrorCaption;

	LOADSTRING( IDS_LOAD_ERROR_CAPTION, strErrorCaption );
	GetMainInfo()->GetCore()->GetFrameWnd()->MessageBox( strErrMsg,
													strErrorCaption );

}

void ChTextOutputWnd::OnTrace( const ChString& strMsg, int iType )
{
	GetMainInfo()->GetCore()->Trace( strMsg, iType == traceError 	
	 									? ChCore::traceErrors  
 										: ChCore::traceWarnings,
 										true );
}



void ChTextOutputWnd::OnHotSpot( chparam userData, const ChString& strDocURL )
{
	char*	pstrCmd;

	if (userData)
	{
		pstrCmd = (char*)userData;
	}
	else
	{
		pstrCmd = "";
	}

	GetMainInfo()->DoHint( pstrCmd );
}


bool ChTextOutputWnd::OnInline( const char* pstrArgs, const ChString& strDocURL )
{
	bool	boolProcessed = false;

	if (pstrArgs)
	{
		ChPane::ChPaneCmd		paneCmd;
											/* Check to see if this is a pane
												inline.  If it is, send it to
												the pane command handler. */

		if (ChPane::GetPaneCmdAttr( pstrArgs, paneCmd ))
		{
			boolProcessed = GetTextOutput()->DoPaneCommand( paneCmd, pstrArgs );
		}
		else
		{									// Otherwise process the inline

			boolProcessed = GetMainInfo()->DoInline( pstrArgs );
		}
	}

	return boolProcessed;
}


void ChTextOutputWnd::DoPopupMenu( CPoint point, const ChString& strWindowName )
{
	m_strMenuWindow = strWindowName;

	#if defined( CH_MSW )
	{
		#if defined( CH_PUEBLO_PLUGIN )
		HINSTANCE	hModule = AfxGetInstanceHandle();
		#else
		HINSTANCE	hModule = ChWorldDLL.hModule;
		#endif
		HINSTANCE	hInstOld = AfxGetResourceHandle();
		CMenu		menuLoaded;
		CMenu		menuPopup;

		AfxSetResourceHandle( hModule );

		#if !defined( CH_PUEBLO_PLUGIN )
			menuLoaded.LoadMenu( ID_MENU_LOG );
		#else
			menuLoaded.LoadMenu( IDR_MAINFRAME );
		#endif
										// Restore the old resource chain
		AfxSetResourceHandle( hInstOld );

		menuPopup.Attach( menuLoaded.GetSubMenu( 0 )->GetSafeHmenu() );

		#if !defined( CH_PUEBLO_PLUGIN )
		chflag32	flEnable;
		bool		boolConnected = GetMainInfo()->IsConnected();
		chint32		lStart;
		chint32		lEnd;


										// Enable & disable items

		menuPopup.EnableMenuItem( ID_MENU_WORLD_ADD,
									boolConnected ? MF_ENABLED : MF_GRAYED );

		GetSel( lStart, lEnd, GetMenuWindow() );
		flEnable = (lStart != lEnd) ? MF_ENABLED : MF_GRAYED;
		menuPopup.EnableMenuItem( ID_EDIT_COPY, flEnable );

		flEnable = ((lStart != lEnd) && boolConnected) ? MF_ENABLED :
															MF_GRAYED;
		menuPopup.EnableMenuItem( ID_EDIT_COPY_TO_INPUT, flEnable );
		#endif

		//{
		//	char buffer[128];
		//	ChString title;
		//	GetWindowText(title);
		//	wsprintf(buffer, "Context menu, window \"%s\" (%08Xh, \"%s\".", (const char *)strWindowName, this, title);
		//	::MessageBox(NULL, buffer, "Debug", MB_OK | MB_ICONINFORMATION);
		//}

										// Display the menu
		ClientToScreen( &point );
		menuPopup.TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON,
									point.x, point.y, this, 0 );
	}
	#else	// defined( CH_MSW )
	{
		#pragma message( "Popup menu not defined for this platform." )
	}
	#endif	// defined( CH_MSW )
}


#ifndef CH_UNIX

BEGIN_MESSAGE_MAP( ChTextOutputWnd, ChHtmlWnd )
	//{{AFX_MSG_MAP(ChTextOutputWnd)
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_KEYDOWN()
#if defined( CH_PUEBLO_PLUGIN )
	ON_WM_INITMENUPOPUP()
	ON_WM_MENUSELECT()
#endif
	//}}AFX_MSG_MAP
	ON_MESSAGE( WM_MODULE_HTTP_REQ_MSG, OnHTTPLoadComplete )
	ON_MESSAGE( WM_EXECUTE_PUEBLO_SCRIPT, OnExecuteScript )
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChTextOutputWnd message handlers
----------------------------------------------------------------------------*/

#if defined( CH_PUEBLO_PLUGIN )
void ChTextOutputWnd::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) 
{
	ChHtmlWnd ::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
	
	// TODO: Add your message handler code here

	if ( 0 == nIndex && pPopupMenu->GetMenuItemCount() > 2 )
	{

		bool		boolConnected = GetMainInfo()->IsConnected();
		chflag32	flEnable;
		chint32		lStart;
		chint32		lEnd;

		pPopupMenu->EnableMenuItem( ID_MENU_WORLD_ADD,
									boolConnected ? MF_ENABLED : MF_GRAYED );

		GetSel( lStart, lEnd, GetMenuWindow() );
		flEnable = (lStart != lEnd) ? MF_ENABLED : MF_GRAYED;
		pPopupMenu->EnableMenuItem( ID_EDIT_COPY, flEnable );

		flEnable = ((lStart != lEnd) && boolConnected) ? MF_ENABLED :
															MF_GRAYED;
		pPopupMenu->EnableMenuItem( ID_EDIT_COPY_TO_INPUT, flEnable );

		flEnable= (!GetMainInfo()->IsLoadPending() &&
									!GetMainInfo()->IsTopLevelWorldList() &&
									!boolConnected )
									? MF_ENABLED : MF_GRAYED;

		pPopupMenu->EnableMenuItem( ID_VIEW_PREVIOUS, flEnable );


		flEnable = GetMainInfo()->GetCore()->GetSplitter()->GetPaneCount() > 1
							? MF_ENABLED : 	MF_GRAYED;
		pPopupMenu->EnableMenuItem( ID_VIEW_TOGGLE_ORIENTATION, flEnable );
		pPopupMenu->EnableMenuItem( ID_VIEW_SWAP_PANES, flEnable );


		pPopupMenu->EnableMenuItem( ID_WORLD_LOG_TO_FILE, 	
				boolConnected ? MF_ENABLED : MF_GRAYED );
		
		if ( boolConnected && GetMainInfo()->GetTextOutput()->IsLogging() )
		{
			pPopupMenu->CheckMenuItem( ID_WORLD_LOG_TO_FILE, MF_CHECKED | MF_BYCOMMAND );
		}

		//pPopupMenu->EnableMenuItem( ID_WORLD_EDIT_PERSONAL, flEnable );
		pPopupMenu->EnableMenuItem( ID_MENU_WORLD_ADD, 
			boolConnected ? MF_ENABLED : MF_GRAYED );

		
		pPopupMenu->EnableMenuItem( ID_WORLD_DISCONNECT, 
			boolConnected ? MF_ENABLED : MF_GRAYED );


	}
	
}

void ChTextOutputWnd::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu) 
{
	ChHtmlWnd ::OnMenuSelect(nItemID, nFlags, hSysMenu);
	
	// TODO: Add your message handler code here
	GetMainInfo()->GetCore()->GetFrameWnd()->
		 SendMessage(  WM_SETMESSAGESTRING, nItemID );
	
}
#endif // #if defined( CH_PUEBLO_PLUGIN )


BOOL ChTextOutputWnd::OnCommand( WPARAM wParam, LPARAM lParam )
{
	bool	boolProcessed = true;

	switch( wParam )
	{
		case ID_EDIT_COPY:
		case ID_EDIT_COPY_TO_INPUT:
		{
			CopyToClipboard( GetMenuWindow() );

			if (ID_EDIT_COPY_TO_INPUT == wParam)
			{
				ASSERT( GetMainInfo()->GetTextInput()->IsShown() );

				GetMainInfo()->GetTextInput()->Paste();
				GetMainInfo()->GetTextInput()->SetFocus();
				SetSel( 0, 0 );
			}
			break;
		}

		case ID_MENU_WORLD_ADD:
		{
			GetMainInfo()->AddCurrentWorld();
			break;
		}

#if defined( CH_PUEBLO_PLUGIN )
		case ID_VIEW_PREVIOUS :
		{
			GetMainInfo()->DoPreviousURL();
			break;
		} 
		case ID_WORLD_EDIT_PERSONAL :
		{
			GetMainInfo()->EditPersonalWorldList();
			break;
		} 
		case ID_WORLD_SHORTCUT :
		{
			if (GetMainInfo()->IsConnected())
			{
				GetMainInfo()->CreateCurrentWorldShortcut();
			}
			else
			{
				GetMainInfo()->CreateShortcut();
			}
			break;
		} 
		case ID_WORLD_DISCONNECT :
		{
			GetMainInfo()->ShutdownWorld();
			break;
		} 
		case ID_WORLD_LOG_TO_FILE :
		{
			GetMainInfo()->GetTextOutput()->ToggleLogging();
			break;
		} 
		case ID_VIEW_TOGGLE_ORIENTATION :
		{
			GetMainInfo()->GetCore()->GetSplitter()->TogglePaneOrientation();
			break;
		} 
		case ID_VIEW_SWAP_PANES :
		{
			GetMainInfo()->GetCore()->GetSplitter()->SwapPanes();
			break;
		} 
		default:
		{
			return GetMainInfo()->GetCore()->GetFrameWnd()->
					SendMessage(  WM_COMMAND, wParam, lParam );
		}
#else
		default:
		{
			boolProcessed = false;
			break;
		}
#endif
	}

	return boolProcessed;
}


void ChTextOutputWnd::OnKillFocus( CWnd* pNewWnd )
{
	ChHtmlWnd::OnKillFocus( pNewWnd );

	GetMainInfo()->SetFocusTarget( focusTextOutput, false );
}


void ChTextOutputWnd::OnSetFocus( CWnd* pOldWnd )
{
	ChHtmlWnd::OnSetFocus( pOldWnd );

	GetMainInfo()->SetFocusTarget( focusTextOutput, true );
}


void ChTextOutputWnd::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
	ChHtmlWnd::OnKeyDown( nChar, nRepCnt, nFlags );

	if ('C' == nChar)
	{
		if (GetKeyState( VK_CONTROL ) & 0x8000)
		{
										// Ctrl + C

			if (GetMainInfo()->GetTextInput()->IsShown())
			{
				GetMainInfo()->GetTextInput()->SetFocus();
				SetSel( 0, 0 );
			}
		}
	}
}


/*----------------------------------------------------------------------------
	This method called when HTTP posts a message 
----------------------------------------------------------------------------*/

LONG ChTextOutputWnd::OnHTTPLoadComplete( UINT wParam, LONG lParam )
{

	ChWorldFileHTTPReq* pLoadInfo = (ChWorldFileHTTPReq*)lParam; 

	if ( pLoadInfo->GetReqID() != GetMainInfo()->GetConnectID()
			|| !GetMainInfo()->IsConnected() )
	{ 	// Too late to use the request
	 	delete pLoadInfo;
		return 0;
	}


	if ( wParam == 0 )
	{
		GetTextOutput()->LoadFile( pLoadInfo->GetFileName(), pLoadInfo->GetURL(),
					pLoadInfo->GetHTML(), pLoadInfo->GetMimeType() );
	}
	else
	{										// Display error message
		ChString		strErrorCaption;

		LOADSTRING( IDS_LOAD_ERROR_CAPTION, strErrorCaption );
		GetMainInfo()->GetCore()->GetFrameWnd()->MessageBox( pLoadInfo->GetErrorMsg(),
														strErrorCaption );
	}

	delete pLoadInfo;

	return 0;
}

/*----------------------------------------------------------------------------
	This method called when onEnhanced mud sends a script  
----------------------------------------------------------------------------*/

LONG ChTextOutputWnd::OnExecuteScript( UINT wParam, LONG lParam )
{
	char* pstrFile = (char*)lParam;

	if ( wParam == 0 )
	{
		ChWorldScript* pWorldScript = new  ChWorldScript( GetMainInfo() );
		ASSERT( pWorldScript );

		pWorldScript->ProcessScript( pstrFile );

		delete pWorldScript;
	}
	delete []pstrFile;

	return 0;
}

#endif // !CH_UNIX

// $Log$
// Revision 1.1.1.1  2003/02/03 18:53:17  uecasm
// Import of source tree as at version 2.53 release.
//
