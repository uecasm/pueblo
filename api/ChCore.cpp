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

	This file contains the implementation of the ChCore class, a base class
	for ChServerCore and ChClientCore.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"


#if !defined( CH_PUEBLO_PLUGIN )
#include "resource.h"
#else
#include "vwrres.h"
#endif

#include <ChTypes.h>
#include <ChConst.h>
#include <ChDispat.h>
#include <ChCore.h>
#include <ChMsgTyp.h>
#include <ChHtmWnd.h>
#include <ChUtil.h>
#include <ChModule.h>
#include <ChReg.h>

#if defined( CH_CLIENT )

	#include <ChPane.h>

#endif // CH_CLIENT

#include <MemDebug.h>

/*----------------------------------------------------------------------------
	class ChTraceWnd
----------------------------------------------------------------------------*/

class ChTraceWnd : public ChHtmlWnd, public ChPaneWndMethods
{
	public:
		ChTraceWnd( ChCore* pCore, ChPane* pPane ) :  
			#if defined( CH_PUEBLO_PLUGIN )
				ChHtmlWnd( pCore->GetHTTPConn() ),
			#endif
				m_pPane( pPane ),
				m_pCore( pCore )
					{
					}

		inline ChCore* GetCore() { return m_pCore; }

		virtual void GetIdealSize( ChSize& size ) { GetDocumentSize( size ); }
		virtual void OnFrameDisconnect( const ChModuleID& idNewModule );
		virtual void OnMouseUp() {}

	protected:
											/* ClassWizard generated virtual
												function overrides */
		//{{AFX_VIRTUAL(ChTextOutputWnd)
		//}}AFX_VIRTUAL

	protected:
		//{{AFX_MSG(ChTextOutputWnd)
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()

	protected:
		ChCore*		m_pCore;
		ChPane*		m_pPane;
};


void ChTraceWnd::OnFrameDisconnect( const ChModuleID& idNewModule )
{
	GetCore()->OnTraceClose();

	DestroyWindow();
	delete this;
}


BEGIN_MESSAGE_MAP( ChTraceWnd, ChHtmlWnd )
	//{{AFX_MSG_MAP(ChTextOutputWnd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChTraceWnd message handlers
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
	ChCore static values
----------------------------------------------------------------------------*/

ChClientInfo	ChCore::m_clientInfo( ChClientInfo::thisMachine );
DWORD			ChCore::m_dwDDEInst = 0;
chflag32		ChCore::m_flTraceOptions = 0;
const chflag32	ChCore::m_flTraceDefault = ChCore::traceHTML |
											ChCore::traceErrors |
											ChCore::traceWarnings |
											ChCore::traceMiscMessages;


/*----------------------------------------------------------------------------
	ChCore class
----------------------------------------------------------------------------*/

ChCore::ChCore() :
			m_pPaneMgr( 0 ),
			m_phookCommand( 0 ),
			m_phookInline( 0 ),
			m_pTracePane( 0 ),
			m_flLastTraceType( 0 )
{
	#if defined( CH_CLIENT )
	{
		UpdateTraceOptions();

		m_pPaneMgr = new ChPaneMgr( this );

		m_phookCommand = new ChHookManager( this );			
		ASSERT( m_phookCommand );

		m_phookInline = new ChHookManager( this );
		ASSERT( m_phookInline );
	}
	#endif
}

ChCore::~ChCore()
{

	#if defined( CH_CLIENT )
	if (0 != m_pPaneMgr)
	{
		delete m_pPaneMgr;
	}

	delete m_phookCommand;			
	delete m_phookInline;

	#endif // CH_CLIENT
}


void ChCore::SetDDEInstance( DWORD dwInst )
{
	m_dwDDEInst = dwInst;
}


void ChCore::OpenTraceWindow()
{
	if (m_pTracePane)
	{
		m_pTracePane->GetWindow()->SetFocus();
	}
	else
	{										/* Need to create the raw data
												window... */

		ChPaneMgr*		pPaneManager = GetPaneMgr();
		chflag32		flOptions = paneOverlapped | paneSizeable |
									paneCloseable | panePersistent;
		ChTraceWnd*		pChild;
		ChRect			rtChild( 10, 10, 300, 500 );
		ChString			strPaneName;

		LOADSTRING( IDS_TRACE_PANE_NAME, strPaneName );

		m_pTracePane = pPaneManager->CreatePane( strPaneName, 0, 300, 100,
													flOptions );
		ASSERT( m_pTracePane );
											/* Create the new child using the
												pane frame window as the
												parent */

		pChild = new ChTraceWnd( this, m_pTracePane );
		ASSERT( pChild );

		pChild->SetBufferLimit( 0x7FFF );				// 32k of text


		#if defined( CH_MSW )

		if (pChild->Create( rtChild, m_pTracePane->GetFrameWnd(),
							WS_VISIBLE | WS_BORDER | WS_VSCROLL ))
		{
			ChString		strPaneTitle;
			ChString		strInitialText;
												// Take ownership of the pane

			m_pTracePane->SetOwner( CH_CORE_MODULE_ID, pChild, pChild );

			LOADSTRING( IDS_TRACE_PANE_TITLE, strPaneTitle );
			m_pTracePane->SetTitle( strPaneTitle );

			ChTraceWnd*	pChild = (ChTraceWnd*)GetTracePane()->GetWindow();

			pChild->NewPage();
			pChild->ShowAppend( true );

			LOADSTRING( IDS_TRACE_PANE_TEXT, strInitialText );
			pChild->AppendText( strInitialText );

												// Finally show the pane
			m_pTracePane->Show();
		}
		else

		#endif	// defined( CH_MSW )

		{
			delete pChild;
			TRACE( "ChTextOutput::DoPaneOpen : Error creating pane.\n" );

			pPaneManager->DestroyPane( strPaneName );
		}
	}
}


void ChCore::DoTrace( const ChString& strText, chflag32 flType, bool boolHtml )
{
	ChTraceWnd*		pChild = (ChTraceWnd*)GetTracePane()->GetWindow();
	chflag32		flOptions;
	ChString			strType;

	if (flType & traceErrors )
	{
		pChild->AppendText( "<p><font fgcolor=\"#FF0000\">" );
		strType = "Error";
	}
	else if (flType & traceWarnings )
	{
		pChild->AppendText( "<p><font fgcolor=\"#0000FF\">" );
		strType = "Warning";
	}
	else if (flType & traceMiscMessages)
	{
		pChild->AppendText( "<p><font fgcolor=\"#008000\">" );
	}

	if (!strType.IsEmpty())
	{
		ChString		strOut;

		strOut = "<b><i>" + strType + ": </i></b>";
		pChild->AppendText( strOut );
	}

	flOptions = boolHtml ? 0 : ChHtmlWnd::modeUntranslated;
	pChild->AppendText( strText, -1, flOptions );

	if (flType & (traceErrors | traceWarnings | traceMiscMessages))
	{
		pChild->AppendText( "</font></p>" );
	}
}


void ChCore::UpdateTraceOptions()
{
	ChRegistry		reg( CH_MISC_GROUP );

	reg.Read( CH_MISC_TRACE_OPTIONS, m_flTraceOptions, m_flTraceDefault );
}


// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
// Revision 1.1.1.1  2003/02/03 18:53:53  uecasm
// Import of source tree as at version 2.53 release.
//
