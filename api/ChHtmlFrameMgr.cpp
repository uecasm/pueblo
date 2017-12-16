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

	This file consists of the implementation of the ChHtmlWnd class.

----------------------------------------------------------------------------*/

#include "headers.h"

#include <ChHtpCon.h>
#include <ChHtmWnd.h>
#include <ChPane.h>

#include "ChHtmlView.h"

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	ChHtmlViewObj class
----------------------------------------------------------------------------*/

ChHtmlViewObj::ChHtmlViewObj( const char* pstrWndName, ChHtmlView* pView,
								CFrameWnd* pFrame ) :
					m_strAltName( pstrWndName),
					m_pHtmlView( pView ),
					m_pFrameWnd( pFrame ),
					m_boolDestroy( true ),
					m_pPaneMgr( 0 )
{
}

ChHtmlViewObj::ChHtmlViewObj( const char* pstrWndName, ChHtmlView* pView,
								ChPaneMgr* pPaneMgr ) :
					m_strAltName( pstrWndName),
					m_pHtmlView( pView ),
					m_pFrameWnd( 0 ),
					m_boolDestroy( true ),
					m_pPaneMgr( pPaneMgr )
{
}


ChHtmlViewObj::~ChHtmlViewObj() 
{
	if (m_boolDestroy)
	{
		if (m_pFrameWnd)
		{
			m_pFrameWnd->DestroyWindow();
		}
		else if (m_pHtmlView)
		{
			ChString strName( m_pHtmlView->GetFrameName() );
			// if we are using the pane manager the delete the frame
			if ( m_pPaneMgr )
			{
				if ( m_pPaneMgr->FindPane( strName ) )
				{
					m_pPaneMgr->DestroyPane( strName );	
				}
			}
			else
			{
				m_pHtmlView->DestroyWindow();
				//delete m_pHtmlView;		// UE: DestroyWindow will delete the object itself.
			}
		}
	}
}


/*----------------------------------------------------------------------------
	ChHtmlWnd class
----------------------------------------------------------------------------*/

ChString ChHtmlWnd::GetAnchorTarget( const char* pstrWindowName )
{
	if (pstrWindowName)
	{
		ChHtmlView*		pView = GetHtmlViewByName( pstrWindowName );

		if (pView)
		{
			return pView->GetAnchorTarget();
		}
		else
		{
			return "";
		}
	}
	
	return GetDefaultView()->GetAnchorTarget();
}


ChString ChHtmlWnd::GetDocURL( const char* pstrWindowName )
{
	if (pstrWindowName)
	{
		ChHtmlView*		pView = GetHtmlViewByName( pstrWindowName );

		if (pView)
		{
			return pView->GetDocURL();
		}
		else
		{
			return "";
		}
	}
	
	return GetDefaultView()->GetDocURL();
}


ChString ChHtmlWnd::GetDocBaseURL( const char* pstrWindowName )
{
	if (pstrWindowName)
	{
		ChHtmlView*		pView = GetHtmlViewByName( pstrWindowName );

		if (pView)
		{
			return pView->GetDocBaseURL();
		}
		else
		{
			return "";
		}
	}

	return GetDefaultView()->GetDocBaseURL();
}


ChString ChHtmlWnd::GetDocTitle( const char* pstrWindowName )
{
	if (pstrWindowName)
	{
		ChHtmlView*		pView = GetHtmlViewByName( pstrWindowName );

		if (pView)
		{
			return pView->GetDocTitle();
		}
		else
		{
			return "";
		}
	}

	return GetDefaultView()->GetDocTitle();
}


void ChHtmlWnd::PageUp( const char* pstrWindowName )
{
	ChHtmlView*		pView;

	if (pstrWindowName)
	{
		pView = GetHtmlViewByName( pstrWindowName );
	}
	else
	{
		pView = GetDefaultView();
	}

	if (pView)
	{
		pView->PageUp();
	}
}


void ChHtmlWnd::PageDown( const char* pstrWindowName )
{
	ChHtmlView*		pView;

	if (pstrWindowName)
	{
		pView = GetHtmlViewByName( pstrWindowName );
	}
	else
	{
		pView = GetDefaultView();
	}

	if (pView)
	{
		pView->PageDown();
	}
}


void ChHtmlWnd::Home( const char* pstrWindowName )
{
	ChHtmlView*		pView;

	if (pstrWindowName)
	{
		pView = GetHtmlViewByName( pstrWindowName );
	}
	else
	{
		pView = GetDefaultView();
	}

	if (pView)
	{
		pView->Home();
	}
}


void ChHtmlWnd::End( const char* pstrWindowName )
{
	ChHtmlView*		pView;

	if (pstrWindowName)
	{
		pView = GetHtmlViewByName( pstrWindowName );
	}
	else
	{
		pView = GetDefaultView();
	}

	if (pView)
	{
		pView->End();
	}
}


bool ChHtmlWnd::IsSelectionEnabled( const char* pstrWindowName )
{
	if (pstrWindowName)
	{
		ChHtmlView*		pView = GetHtmlViewByName( pstrWindowName );

		if (pView)
		{
			return pView->IsSelectionEnabled();
		}
		else
		{
			return false;
		}
	}

	return GetDefaultView()->IsSelectionEnabled( );
}


void ChHtmlWnd::EnableSelection( bool boolSel, const char* pstrWindowName )
{
	if (pstrWindowName)
	{
		ChHtmlView*		pView = GetHtmlViewByName( pstrWindowName );

		if (pView)
		{
			pView->EnableSelection( boolSel );
		}
	}
	else
	{
		GetDefaultView()->EnableSelection( boolSel );
	}
}


void ChHtmlWnd::GetSel( chint32& lStart, chint32& lEnd,
						const char* pstrWindowName )
{
	if (pstrWindowName)
	{
		ChHtmlView*		pView = GetHtmlViewByName( pstrWindowName );

		if (pView)
		{
			pView->GetSel( lStart, lEnd );
		}
	}
	else
	{
		GetDefaultView()->GetSel( lStart, lEnd );
	}
}


void ChHtmlWnd::SetSel( chint32 lStart, chint32 lEnd,
						const char* pstrWindowName )
{
	if (pstrWindowName)
	{
		ChHtmlView*		pView = GetHtmlViewByName( pstrWindowName );

		if (pView)
		{
			pView->SetSel( lStart, lEnd );
		}
	}
	else
	{
		GetDefaultView()->SetSel( lStart, lEnd );
	}
}


void ChHtmlWnd::CopyToClipboard( const char* pstrWindowName )
{
	if (pstrWindowName)
	{
		ChHtmlView*		pView = GetHtmlViewByName( pstrWindowName );

		if (pView)
		{
			pView->CopyToClipboard();
		}
	}
	else
	{
		GetDefaultView()->CopyToClipboard();
	}
}


void ChHtmlWnd::SetAnchorTarget( const ChString& strTarget,
									const char* pstrWindowName )
{
	if (pstrWindowName)
	{
		ChHtmlView*	pView = GetHtmlViewByName( pstrWindowName );

		if (pView)
		{
			pView->SetAnchorTarget( strTarget );
		}
	}
	else
	{
		GetDefaultView()->SetAnchorTarget( strTarget );
	}
}


chint32 ChHtmlWnd::GetBufferLimit( const char* pstrWindowName )
{
	if (pstrWindowName)
	{
		ChHtmlView*		pView = GetHtmlViewByName( pstrWindowName );

		if (pView)
		{
			return pView->GetBufferLimit();
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return GetDefaultView()->GetBufferLimit();
	}
}


bool ChHtmlWnd::GetAppend( const char* pstrWindowName )
{
	if (pstrWindowName)
	{
		ChHtmlView*		pView = GetHtmlViewByName( pstrWindowName );

		if (pView)
		{
			return pView->GetAppend();
		}
		else
		{
			return false;
		}
	}
	else
	{
		return GetDefaultView()->GetAppend();
	}
}


void ChHtmlWnd::SetBufferLimit( chint32 lLimit, const char* pstrWindowName )
{
	if (pstrWindowName)
	{
		ChHtmlView*		pView = GetHtmlViewByName( pstrWindowName );

		if (pView)
		{
			pView->SetBufferLimit( lLimit );
		}
	}
	else
	{
		GetDefaultView()->SetBufferLimit( lLimit );
	}
}


void ChHtmlWnd::ShowAppend( bool boolAppend, const char* pstrWindowName )
{
	if (pstrWindowName)
	{
		ChHtmlView*		pView = GetHtmlViewByName( pstrWindowName );

		if (pView)
		{
			pView->ShowAppend( boolAppend );
		}
	}
	else
	{
		GetDefaultView()->ShowAppend( boolAppend );
	}
}


void ChHtmlWnd::GetScrollState( bool& boolHorizontal, bool& boolVertical,
								const char* pstrWindowName )
{
	if (pstrWindowName)
	{
		ChHtmlView*		pView = GetHtmlViewByName( pstrWindowName );

		if (pView)
		{
			pView->GetScrollState( boolHorizontal,  boolVertical );
		}
	}
	else
	{
		GetDefaultView()->GetScrollState( boolHorizontal, boolVertical );
	}
}


void ChHtmlWnd::AllowScroll( bool boolHorizontal, bool boolVertical,
								const char* pstrWindowName )
{
	if (pstrWindowName)
	{
		ChHtmlView*		pView = GetHtmlViewByName( pstrWindowName );

		if (pView)
		{
			pView->AllowScroll( boolHorizontal, boolVertical );
		}
	}
	else
	{
		GetDefaultView()->AllowScroll( boolHorizontal, boolVertical );
	}
}


void ChHtmlWnd::GetDocumentSize( ChSize& docSize, const char* pstrWindowName )
{
	if (pstrWindowName)
	{
		ChHtmlView*		pView = GetHtmlViewByName( pstrWindowName );

		if (pView)
		{
			pView->GetDocumentSize( docSize );
		}
	}
	else
	{
		GetDefaultView()->GetDocumentSize( docSize );
	}
}


void ChHtmlWnd::GetViewIndents( ChRect& viewIndents,
								const char* pstrWindowName )
{
	if (pstrWindowName)
	{
		ChHtmlView*		pView = GetHtmlViewByName( pstrWindowName );

		if (pView)
		{
			pView->GetViewIndents( viewIndents );
		}
	}
	else
	{
		GetDefaultView()->GetViewIndents( viewIndents );
	}
}

void ChHtmlWnd::SetViewIndents( const ChRect& viewIndent,
								const char* pstrWindowName )
{
	if (pstrWindowName)
	{
		ChHtmlView*		pView = GetHtmlViewByName( pstrWindowName );

		if (pView)
		{
			pView->SetPageIndents( viewIndent );
		}
	}
	else
	{
		GetDefaultView()->SetPageIndents( viewIndent );
	}
}


chuint32 ChHtmlWnd::GetDocumentAttrs( const char* pstrWindowName )
{
	if (pstrWindowName)
	{
		ChHtmlView*		pView = GetHtmlViewByName( pstrWindowName );

		if (pView)
		{
			return pView->GetDocumentAttrs();
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return GetDefaultView()->GetDocumentAttrs();
	}
}


void ChHtmlWnd::SetDocumentAttrs( const chuint32 luAttrs,
									const char* pstrWindowName )
{
	if (pstrWindowName)
	{
		ChHtmlView*		pView = GetHtmlViewByName( pstrWindowName );

		if (pView)
		{
			pView->SetDocumentAttrs( luAttrs );
		}
	}
	else
	{
		GetDefaultView()->SetDocumentAttrs( luAttrs );
	}
}


bool ChHtmlWnd::WriteFile( const char* pstrFilePath, chflag32 flOptions,
							const char* pstrWindowName )
{
	if (pstrWindowName)
	{
		ChHtmlView*		pView = GetHtmlViewByName( pstrWindowName );

		if (pView)
		{
			return pView->WriteFile( pstrFilePath, flOptions );
		}
		else
		{
			return false;
		}
	}
	else
	{
		return GetDefaultView()->WriteFile( pstrFilePath, flOptions );
	}
}


bool ChHtmlWnd::CloseFile( const char* pstrWindowName )
{
	if (pstrWindowName)
	{
		ChHtmlView*		pView = GetHtmlViewByName( pstrWindowName );

		if (pView)
		{
			return pView->CloseFile();
		}
		else
		{
			return false;
		}
	}
	else
	{
		return GetDefaultView()->CloseFile();
	}
}

void ChHtmlWnd::RemapColors( int iNumColors, 
						chuint32* pluOldColor, chuint32* pluNewColors, 
						const char* pstrWindowName /* = 0 */ )
{
	if (pstrWindowName)
	{
		ChHtmlView*		pView = GetHtmlViewByName( pstrWindowName );

		if (pView)
		{
			pView->RemapColors( iNumColors, pluOldColor, pluNewColors );
		}
	}
	else
	{
		GetDefaultView()->RemapColors( iNumColors, pluOldColor, 
								pluNewColors );
	}
}

void ChHtmlWnd::PostMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	GetDefaultView()->PostMessage(WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(pt.x, pt.y));
}

ChHtmlView* ChHtmlWnd::GetDefaultView()
{
											/* View at the head is always the
												default view */

	return m_htmlViewList.GetHead()->GetHtmlView();
}


ChHtmlView* ChHtmlWnd::GetHtmlViewByName( const ChString& strName )
{
	ChPosition pos = m_htmlViewList.GetHeadPosition();

	while (pos)
	{
		ChHtmlViewObj*	pFrameObj = m_htmlViewList.GetNext( pos );

		if (0 == pFrameObj->GetHtmlView()->GetFrameName().CompareNoCase( strName ))
		{
			return pFrameObj->GetHtmlView();
		}
		else if (0 == pFrameObj->GetAltFrameName().CompareNoCase( strName ))
		{
			return pFrameObj->GetHtmlView();
		}
	}

	return 0;
}


void ChHtmlWnd::SetFocus( const char* pstrWindowName )
{
	if (pstrWindowName)
	{
		ChHtmlView*		pView = GetHtmlViewByName( pstrWindowName );

		if (pView)
		{
			pView->SetFocus();
		}
	}
	else
	{
		GetDefaultView()->SetFocus();
	}
}


ChHtmlView* ChHtmlWnd::CreateHtmlView( const char* pstrName,
												CWnd* pFrame )
{
	const char*		pstrWndName;
	ChHtmlView*		pView = 0;
	CWnd*			pParent;
	bool			boolAdd = false;

	ASSERT( pstrName );

	TRACE("ChHtmlWnd::CreateHtmlView(\"%s\", %08Xh)\n", pstrName, pFrame);	// UE DEBUG

	if (m_htmlViewList.GetCount() == 1)
	{										// main view
		pstrWndName = TOP_WINDOW;
		pView = m_htmlViewList.GetHead( )->GetHtmlView();
		if ( pView->GetSafeHwnd() )
		{
			pView = 0;
		}
	}
	
	if ( pView == 0 )
	{
		TRACE("Creating view object\n");	// UE DEBUG
		pstrWndName = pstrName;
		pView = new ChHtmlView( pstrWndName, this );
		ASSERT( pView );	
		boolAdd = true;
	}

	if (pFrame)
	{
		pParent = pFrame;
	}
	else
	{
		pParent = (CWnd*)this;
	}

	CRect			rtView;  

	pParent->GetClientRect( &rtView );
	if (pView->Create( rtView,  pParent, WS_CHILD | WS_VISIBLE,
						m_htmlViewList.GetCount() ))
	{
											// Add it to our list
		if ( boolAdd )
		{
		 	ChHtmlViewObj*	pFrameObj;

			if ( m_pPaneMgr )
			{
			 	pFrameObj = new ChHtmlViewObj( ChString( pstrName ), pView, (ChPaneMgr*)m_pPaneMgr );
			}
			else
			{
			 	pFrameObj = new ChHtmlViewObj( ChString( pstrName ), pView, (CFrameWnd*)pFrame );
			}
			ASSERT( pFrameObj );
			m_htmlViewList.AddTail( pFrameObj );
		}

		return pView;
	}

	delete pView;
	return 0;
}


void ChHtmlWnd::RecalcLayout()
{
	CRect rtClient;

	GetClientRect( &rtClient );

	m_htmlViewList.GetHead()->GetHtmlView()->
			SetWindowPos( 0, 0, 0, rtClient.Width(), rtClient.Height(),
							SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
}
