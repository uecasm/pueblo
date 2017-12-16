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

	This file consists of the implementation of the ChHtmlPane  class.

----------------------------------------------------------------------------*/

#include "headers.h"

#include <ChModule.h>
#include <ChHtmWnd.h>
#include <ChPane.h>

#include "ChHtmlView.h"
#include "ChHtmlStream.h"
#include "ChHtmlPane.h"
#include "ChHtmSym.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	ChHtmlPane class
----------------------------------------------------------------------------*/

ChHtmlPane::ChHtmlPane() :
				m_iAction( actionOpen ),
				m_luOption( optionOverlapped ), 
			 	m_iVSpace( 0 ),
			 	m_iHSpace( 0 ),
			 	m_iWidth( 300 ),
			 	m_iHeight( 200 ),
				m_iMinWidth( 0 ),
				m_iMinHeight( 0 ),
				m_iScroll( scrollAuto ),
				m_iAlign( 0 )
{
}


/*----------------------------------------------------------------------------
	ChHtmlWnd class
----------------------------------------------------------------------------*/

 void ChHtmlWnd::CreatePane( ChHtmlView* pView, ChHtmlPane* pPane )
 {
	if (pPane->GetAction() == ChHtmlPane::actionMove)
	{
											//Not handled currently
	}
	else if (pPane->GetAction() == ChHtmlPane::actionClose)
	{ 
											// Close this pane

		if (pPane->GetName() == pView->GetFrameName())
		{
			TRACE( "Cannot close an active pane\n" );
		}
		else
		{
			ClosePane( pPane->GetName() );
		}
	}
	else if (pPane->GetAction() == ChHtmlPane::actionOpen)
	{ 
		//TRACE0("Pane: action == open\n");
		if (pPane->GetName().IsEmpty() || pPane->GetName() == BLANK_WINDOW )
		{
			//TRACE0("Pane: name == _blank (or undefined)\n");
 			if ( pPane->GetOptions() & ChHtmlPane::optionBrowser || 
 						pPane->GetOptions() & ChHtmlPane::optionWebTracker) 
			{
				//TRACE0("Pane: options & (browser | webtracker)\n");
				pPane->SetName( BLANK_WINDOW );
			}
			else
			{
				//TRACE0("Pane: options & ~(browser | webtracker)\n");
				ChString		strTarget;
				strTarget.Format( "%s%ld", BLANK_WINDOW, ::GetMessageTime() );
				pPane->SetName( strTarget );

				pPane->SetOptions( pPane->GetOptions() & ~ChHtmlPane::optionNoClose );
			}
		}
	 	

	 	if (pPane->GetName() == BLANK_WINDOW)
	 	{
 			if ( pPane->GetOptions() & ChHtmlPane::optionBrowser || 
 						pPane->GetOptions() & ChHtmlPane::optionWebTracker) 
 			{
 				//TRACE1("Pane: calling OnRedirect(\"%s\", ...)\n",
 				//				(LPCSTR)pPane->GetURL());
 				OnRedirect( pPane->GetURL(),
 						 (pPane->GetOptions() & ChHtmlPane::optionWebTracker) != FALSE );
			}			
	 	}
		else if ( !pPane->GetName().IsEmpty() && !pPane->GetURL().IsEmpty() &&
					pPane->GetName() != PREVIOUS_WINDOW )
		{
			ChString		strTarget( pPane->GetName() == SELF_WINDOW 
												? pView->GetFrameName()
												: pPane->GetName() );
			ChHtmlView* pTargetView = GetHtmlViewByName(  strTarget );	

			if ( !pTargetView )
			{
			 	if ( pPane->GetOptions() & ChHtmlPane::optionExisting )
				{
					//TRACE0("Pane: options & existing, but not existing.\n");
					return;
				}
				//TRACE1("Pane: creating view (\"%s\")\n", (LPCSTR)pPane->GetURL());
				// create the frame and the view window
				pTargetView = CreateView( pPane );
				ASSERT( pTargetView );
		
				pTargetView->NewPage();
				pTargetView->SetDocURL( pPane->GetURL() );
				pTargetView->SetDocBaseURL( pPane->GetURL() );
			}

			ASSERT( pTargetView );

			if ( ChHtmlPane::optionViewBottom & pPane->GetOptions() )
			{
				pTargetView->ShowAppend( true );
			}
			else
			{
				pTargetView->ShowAppend( false );
			}

			//TRACE0("Pane: pane valid, loading URL.\n");

			ChHtmlFrameReq* pReq = new ChHtmlFrameReq( pTargetView, 
										pPane->GetOptions() );
			ASSERT( pReq );
			LoadURL( pPane->GetURL(), pReq );
		}
		else
		{ // bogus option for action=open
			//TRACE0("Pane: action=open bogus option.\n");
			return;
		}
	}
	else if (pPane->GetAction() == ChHtmlPane::actionRedirect)
	{
		if (pPane->GetName().IsEmpty())
		{
			return;
		}
	 	
	 	if ( pPane->GetName() == BLANK_WINDOW )
	 	{
			ChString		strTarget;
			strTarget.Format( "%s%ld", BLANK_WINDOW, ::GetMessageTime() );
			pPane->SetName( strTarget );
	 	}
		
		if ( pPane->GetName() == PREVIOUS_WINDOW || pPane->GetName() == TOP_WINDOW  )
		{
			if ( pPane->GetURL().IsEmpty() )
			{
				RedirectStream( pView->GetFrameName(), 	pPane->GetName() );
			}
			else
			{ // redirect the URL to the previous pane
				RedirectURL( pPane->GetName(),	pPane->GetURL(), pPane->GetOptions() );
			}
		}
		else if ( pPane->GetName() != SELF_WINDOW )
		{
			ChHtmlView* pTargetView = GetHtmlViewByName(  pPane->GetName() );	

			if ( !pTargetView )
			{
			 	if ( pPane->GetOptions() & ChHtmlPane::optionExisting )
				{
					// Consume everything till the next xch_pane
					if ( pPane->GetURL().IsEmpty() )
					{
						RedirectStream( pView->GetFrameName(), 	pPane->GetName() );
					}
					else
					{ // continue streaming to the current view
					}
					return;
				}
				// create the frame and the view window	 
				pTargetView = CreateView( pPane );

				if ( pPane->GetURL().IsEmpty() )
				{
					// Set the defaults
					pTargetView->NewPage();
					pTargetView->SetDocURL( pView->GetDocURL() );
					pTargetView->SetDocBaseURL( pView->GetDocBaseURL() );
				}
				else
				{
					// Set the defaults
					pTargetView->NewPage();
					pTargetView->SetDocURL( pPane->GetURL() );
					pTargetView->SetDocBaseURL( pPane->GetURL() );
				}

			}
			ASSERT( pTargetView );

			if ( ChHtmlPane::optionViewBottom & pPane->GetOptions() )
			{
				pTargetView->ShowAppend( true );
			}
			else
			{
				pTargetView->ShowAppend( false );
			}

			if ( pPane->GetURL().IsEmpty() )
			{
				RedirectStream( pView->GetFrameName(), pPane->GetName() );
			}
			else
			{
				RedirectURL( pTargetView->GetFrameName(), pPane->GetURL(), pPane->GetOptions() );
			}
		}
		else if ( !pPane->GetURL().IsEmpty() )
		{
			RedirectURL( pView->GetFrameName(),	pPane->GetURL(), pPane->GetOptions() );
		}
	}
} 
 

ChHtmlView* ChHtmlWnd::CreateView( ChHtmlPane* pInfo )
{
   	ChHtmlView*		pView = 0;

	if (m_pPaneMgr)
	{										// Use the external pane manager
		ChPane*		pPane = 0;

		if (!m_pPaneMgr->FindPane( pInfo->GetName() ))
		{
			chuint32	flPaneOptions = 0;
			
			if (pInfo->GetOptions() & ChHtmlPane::optionOverlapped)
			{
			 	flPaneOptions |= paneOverlapped;
			}
			else if (pInfo->GetOptions() & ChHtmlPane::optionFloating)
			{
			 	flPaneOptions |= paneFloat;
			}
			else if (pInfo->GetOptions() & ChHtmlPane::optionInternal)
			{
			 	flPaneOptions |= paneInternal;

				if (pInfo->GetOptions() & ChHtmlPane::optionDocking)
				{
					if (pInfo->GetOptions() & ChHtmlPane::optionDocked)
					{
					}
				}
			}

			if ( !(pInfo->GetOptions() & ChHtmlPane::optionNoClose) )
			{
				flPaneOptions |= paneCloseable;
			}

			if ( !(pInfo->GetOptions() & ChHtmlPane::optionNonSizeable) )
			{
				flPaneOptions |= paneSizeable;
			}

			if (pInfo->GetOptions() & ChHtmlPane::optionPersistent)
			{
				flPaneOptions |= panePersistent;

				if (pInfo->GetOptions() & ChHtmlPane::optionForce)
				{
					flPaneOptions |= paneForce;
				}
			}

			if (pInfo->GetOptions() & ChHtmlPane::optionSmallTitle)
			{
				flPaneOptions |= paneSmallTitle;
			}

			if (pInfo->GetOptions() & ChHtmlPane::optionFit)
			{
				flPaneOptions |= paneSizeToFit;
			}

			if (pInfo->GetScrolling() == ChHtmlPane::scrollNo)
			{
				flPaneOptions |= paneNoScroll;
			}

			if (pInfo->GetAlign() == VAL_TOP)
			{
				flPaneOptions |= paneAlignTop;
			}

			if (pInfo->GetAlign() == VAL_BOTTOM)
			{
				flPaneOptions |= paneAlignBottom;
			}

			TRACE("Creating pane\n");	// UE DEBUG
		
			pPane = m_pPaneMgr->CreatePane( pInfo->GetName(), 0,
											pInfo->GetWidth(),
											pInfo->GetHeight(),
											flPaneOptions );
			ASSERT( pPane );
		}
		else
		{
			TRACE( "There is already a pane with the given name\n" );
			ASSERT( false );
			return 0;
		}
											/* Create the new child using the
												pane frame window as the
												parent */

		pView = CreateHtmlView( pInfo->GetName(), pPane->GetFrameWnd() );
		ASSERT( pView );

		if (pView)
		{
			TRACE("Created view\n");	// UE DEBUG

			if (pInfo->GetOptions() & ChHtmlPane::optionInternal)
			{
				pView->SetEmbedMode( ChHtmlView::embedInternal );
			}
			else
			{
				pView->SetEmbedMode( ChHtmlView::embedFloat );
			}
												/* Take ownership of the pane.
													We're using CH_CORE_MODULE_ID
													here so we can tell if
													the module changes to zero.
													This is weird, and should be
													fixed somehow. */

			pPane->SetOwner( CH_CORE_MODULE_ID, pView, pView );
			pPane->SetTitle( pInfo->GetTitle() );
			pPane->SetSizePrefs( pInfo->GetWidth(), pInfo->GetHeight(),
									pInfo->GetMinWidth() ?
										pInfo->GetMinWidth() : 20,
									pInfo->GetMinHeight() ?
										pInfo->GetMinHeight() : 20 );

			if (!pPane->IsSizeable() || !pPane->IsSized())
			{
				pPane->SetSize(  pInfo->GetWidth(), pInfo->GetHeight() );
			}
												/* Set view indents, if
													necessary */
			if (pInfo->GetScrolling() == ChHtmlPane::scrollNo )
			{
				CRect	rtEdges( 0, 0, 0, 0 );

				pView->AllowScroll( false, false );
				pView->SetPageIndents( rtEdges );
			}
			else
			{
				ChRect		rtIndent;

				rtIndent.left = pInfo->GetHSpace();
				rtIndent.top = pInfo->GetVSpace();
				rtIndent.right = rtIndent.bottom = 0;
	
				pView->SetPageIndents( rtIndent );
			}
		}

		if (pInfo->GetAction() == ChHtmlPane::actionRedirect)
		{
			pPane->Show();
		}
	}
	else
	{
		DWORD dwStyle = WS_OVERLAPPED | WS_BORDER;

		if ( !(pInfo->GetOptions() & ChHtmlPane::optionNoClose) )
		{
			dwStyle |= WS_SYSMENU;
		}
		if (pInfo->GetOptions() & ChHtmlPane::optionNonSizeable)
		{
			dwStyle |= WS_MINIMIZEBOX;
		}
		else
		{
			dwStyle |= WS_SIZEBOX | WS_MINIMIZEBOX  | WS_MAXIMIZEBOX;
		}

		CRect rtPos;
		GetWindowRect( &rtPos );
		CRect rtSize( rtPos.left, rtPos.top, 300, 200 );


		rtSize.right = pInfo->GetWidth();
		rtSize.bottom = pInfo->GetHeight();

		TRACE("Couldn't create view, creating frame instead\n");	// UE DEBUG

		if ( pInfo->GetOptions() & ChHtmlPane::optionSmallTitle )
		{
			ChHtmlMiniFrame* pFrame = new ChHtmlMiniFrame( this, pInfo->GetName(),
										(pInfo->GetOptions() & ChHtmlPane::optionFit) != FALSE  );
			ASSERT( pFrame );
			
			if ( pFrame->Create( NULL, pInfo->GetTitle(), dwStyle, rtSize ) )
			{
				pView = pFrame->GetHtmlView();
			}
		}
		else
		{
			ChHtmlFrame* pFrame = new ChHtmlFrame( this, pInfo->GetName(),
										(pInfo->GetOptions() & ChHtmlPane::optionFit) != FALSE );
			ASSERT( pFrame );	
			
			if ( pFrame->Create( NULL, pInfo->GetTitle(), dwStyle, rtSize ) )
			{
				pView = pFrame->GetHtmlView();
			}
		}
		if (pInfo->GetScrolling() == ChHtmlPane::scrollNo )
		{
			CRect	rtEdges( 0, 0, 0, 0 );

			pView->AllowScroll( false, false );
			pView->SetPageIndents( rtEdges );
		}

		if (  pInfo->GetAction() == ChHtmlPane::actionRedirect  )
		{
			pView->GetParent()->ShowWindow( SW_SHOWNORMAL );
		}
		pView->SetEmbedMode( ChHtmlView::embedFloat );

	}

	return pView;
}


void ChHtmlWnd::CloseAllSubPanes()
{
	while ( m_pRedirectStack )
	{
		while( m_pRedirectStack->GetCount() )
		{
			delete  m_pRedirectStack->RemoveHead();
		}
		delete m_pRedirectStack;
		m_pRedirectStack = 0;		
	}

 	while( m_htmlViewList.GetCount() > 1 )
 	{
		ChHtmlViewObj*	pFrameObj = m_htmlViewList.GetTail( );
		ClosePane( pFrameObj->GetHtmlView()->GetFrameName() );
 	}  	
}

 void ChHtmlWnd::ClosePane( const ChString& strName, bool boolDestroy /* = true */ )
 {
 	ChPosition pos = m_htmlViewList.GetHeadPosition();

	if ( pos )
	{
		m_htmlViewList.GetNext( pos );
	}

	while( pos != 0 )
	{
		ChPosition posDel = pos;

		ChHtmlViewObj*	pFrameObj = m_htmlViewList.GetNext( pos );

		if ( pFrameObj->GetHtmlView()->GetFrameName().CompareNoCase( strName ) == 0 )
		{
			m_htmlViewList.Remove( posDel );

			pFrameObj->SetDestroy( boolDestroy );
			delete pFrameObj;
			pos = 0;
		}
		else if ( pFrameObj->GetAltFrameName().CompareNoCase( strName ) == 0 )
		{
			m_htmlViewList.Remove( posDel );
			pFrameObj->SetDestroy( boolDestroy );
			delete pFrameObj;
			pos = 0;
		}
	}

	return;
}

bool ChHtmlWnd::IsRedirectStream( const ChString& strCurrStream  )
{
	if ( m_pRedirectStack && m_pRedirectStack->GetCount() )
	{
		ChRedirectInfo *pRedirecInfo = m_pRedirectStack->GetHead();
		if ( pRedirecInfo->GetTarget() != strCurrStream )
		{
			return true;
		}
	}
	return false;
}


bool ChHtmlWnd::RedirectStream( const ChString& strCurrStream, const ChString& strTarget )
{
	ChString strRedirect;

 	if ( strTarget.IsEmpty() || strTarget == SELF_WINDOW || strTarget == strCurrStream )
 	{
		strRedirect = strCurrStream;	
 	} 
 	else if ( strTarget == PREVIOUS_WINDOW )
	{
		strRedirect = PREVIOUS_WINDOW;	
	}
 	else if ( strTarget == TOP_WINDOW )
	{
		strRedirect = TOP_WINDOW;	
	}
	else
	{
		strRedirect = strTarget;
	}

	if ( 0== m_pRedirectStack )
	{
		m_pRedirectStack = new ChHTMLRedirectStack();
		ASSERT( m_pRedirectStack );	
	}

 	if ( strRedirect == PREVIOUS_WINDOW )
	{
		if ( m_pRedirectStack->GetCount() == 0 )
		{
			delete m_pRedirectStack;
			m_pRedirectStack = 0;
			return true;
		}
		else
		{
			if ( m_pRedirectStack->GetCount() == 1 )
			{
				delete	m_pRedirectStack->RemoveHead();
				strRedirect = TOP_WINDOW;

				if ( strRedirect != strCurrStream )
				{
					ChRedirectInfo *pRedirecInfo = new ChRedirectInfo( strRedirect, strCurrStream );
					ASSERT( pRedirecInfo );
					m_pRedirectStack->AddHead( pRedirecInfo ); 
				}
				else
				{
					while( m_pRedirectStack->GetCount() )
					{
						delete  m_pRedirectStack->RemoveHead();
					}
					delete m_pRedirectStack;
					m_pRedirectStack = 0;
					return true;
				}
			}
			else
			{
			 	ChRedirectInfo *pRedirecInfo = m_pRedirectStack->RemoveHead();
				delete pRedirecInfo;
			}
				
		}
		return true;
	}
	else if ( strRedirect == TOP_WINDOW )
	{
		// Empty the whole stack
		while( m_pRedirectStack->GetCount() )
		{
			delete  m_pRedirectStack->RemoveHead();
		}

		if ( strCurrStream == strRedirect )
		{  

		   delete  m_pRedirectStack;
		   m_pRedirectStack = 0;
		}
		else
		{
			ChRedirectInfo *pRedirecInfo = new ChRedirectInfo( TOP_WINDOW, strCurrStream );
			ASSERT( pRedirecInfo );
			m_pRedirectStack->AddHead( pRedirecInfo ); 
		}
	}
	else 
	{
		if ( m_pRedirectStack->GetCount() )
		{
		 	ChRedirectInfo *pRedirecInfo = m_pRedirectStack->GetHead();
			if ( strRedirect == pRedirecInfo->GetTarget() )
			{
				TRACE( "Already redirected !!!\n" );
				return false;
			}
		}
		ChRedirectInfo *pRedirecInfo = new ChRedirectInfo( strRedirect, strCurrStream );
		ASSERT( pRedirecInfo );
		m_pRedirectStack->AddHead( pRedirecInfo ); 
	}
	return true;
}

bool ChHtmlWnd::RedirectURL( const ChString& strTarget, const ChString& strURL, chuint32 flOptions )
{

	ChHtmlView* pView = 0;

 	if ( strTarget == PREVIOUS_WINDOW )
	{
		if ( m_pRedirectStack->GetCount() > 1 )
		{
			ChPosition pos = m_pRedirectStack->GetTailPosition();
			m_pRedirectStack->GetPrev( pos );
			if( pos )
			{
				ChRedirectInfo *pRedirecInfo = m_pRedirectStack->Get( pos );
				pView = GetHtmlViewByName( pRedirecInfo->GetTarget()  );	
			}
		}
		else
		{
			pView = GetDefaultView();
		}
	}
 	else if ( strTarget == TOP_WINDOW )
	{
		pView = GetDefaultView();
	}
	else
	{
		pView = GetHtmlViewByName( strTarget );
	}

	if ( pView )
	{
		ChHtmlFrameReq* pReq = new ChHtmlFrameReq( pView, 
									flOptions 
									| ChHtmlPane::optionFileAppend);
		ASSERT( pReq );
		LoadURL( strURL, pReq );
	}
	return pView != 0;
}




/////////////////////////////////////////////////////////////////////////////
// ChHtmlFrame


ChHtmlFrame::ChHtmlFrame( ChHtmlWnd* pFrameMgr, const ChString& strName, bool boolSizeToFit )	:
						CFrameWnd(),
						m_pFrameMgr( pFrameMgr ), 
						m_strName( strName ), 
						m_boolSizeToFit( boolSizeToFit ),
						m_pView( 0 )
{
}

ChHtmlFrame::~ChHtmlFrame()
{
	delete m_pView;
}


BEGIN_MESSAGE_MAP(ChHtmlFrame, CFrameWnd)
	//{{AFX_MSG_MAP(ChHtmlFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// ChHtmlFrame message handlers

int ChHtmlFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	m_pView=  GetFrameMgr()->CreateHtmlView( GetFrameName(),  this );
	if ( !m_pView )
	{
		return -1;
	}
	
	return 0;
}

void ChHtmlFrame::OnDestroy() 
{
	CFrameWnd::OnDestroy();
	
	// TODO: Add your message handler code here
	GetFrameMgr()->ClosePane( GetFrameName(), false );
	m_pView->DestroyWindow();
	
}

void ChHtmlFrame::OnSize(UINT nType, int cx, int cy) 
{
	CFrameWnd::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	m_pView->SetWindowPos( 0, 0, 0, cx,	cy, 
							SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
}
/////////////////////////////////////////////////////////////////////////////
// ChHtmlMiniFrame


ChHtmlMiniFrame::ChHtmlMiniFrame(ChHtmlWnd* pFrameMgr, const ChString& strName, bool boolSizeToFit )	:
						CMiniFrameWnd(),
						m_pFrameMgr( pFrameMgr ), 
						m_strName( strName ), 
						m_boolSizeToFit( boolSizeToFit ),
						m_pView( 0 )
{
}

ChHtmlMiniFrame::~ChHtmlMiniFrame()
{
	delete m_pView;
}


BEGIN_MESSAGE_MAP(ChHtmlMiniFrame, CMiniFrameWnd)
	//{{AFX_MSG_MAP(ChHtmlMiniFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// ChHtmlMiniFrame message handlers

int ChHtmlMiniFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMiniFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	m_pView=  GetFrameMgr()->CreateHtmlView( GetFrameName(),  this );
	if ( !m_pView )
	{
		return -1;
	}
	
	return 0;
}

void ChHtmlMiniFrame::OnDestroy() 
{
	CMiniFrameWnd::OnDestroy();
	
	// TODO: Add your message handler code here
	GetFrameMgr()->ClosePane( GetFrameName(), false );
	m_pView->DestroyWindow();
}

void ChHtmlMiniFrame::OnSize(UINT nType, int cx, int cy) 
{
	CMiniFrameWnd::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	m_pView->SetWindowPos( 0, 0, 0, cx,	cy, 
							SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
	
}
