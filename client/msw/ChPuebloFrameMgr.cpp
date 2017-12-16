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

This file includes the code for the main Pueblo client app frame manager.

----------------------------------------------------------------------------*/

#include "headers.h"


#include "Pueblo.h"
#include "ChMFrame.h"
#include "ChClCore.h"
#include "MemDebug.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif


/*----------------------------------------------------------------------------
	ChVisitFind class
----------------------------------------------------------------------------*/

class ChVisitFind : public ChPtrSplayVisitor2<ChMainFrame>
{
	public:
		ChVisitFind( const char* pstrFrame ) :
							m_pstrFrameName( pstrFrame ),
							m_pFrame( 0 )
				{
				}

		virtual bool Visit( chparam key, const ChMainFrame* pFrame );

		inline const ChMainFrame* GetFrame() const { return m_pFrame; }

	private :
		const char*			m_pstrFrameName;
		const ChMainFrame*	m_pFrame;
};


bool ChVisitFind::Visit( chparam key, const ChMainFrame* pFrame  )
{
	ChString		strLabel = ((ChMainFrame* const)pFrame)->GetLabel();

	if (0 == strLabel.CompareNoCase( m_pstrFrameName ))
	{
		m_pFrame = pFrame;
		return false;
	}
	return true;
}


/*----------------------------------------------------------------------------
	ChVisitGetNext class
----------------------------------------------------------------------------*/

class ChVisitGetNext : public ChPtrSplayVisitor2<ChMainFrame>
{
	public:
		ChVisitGetNext( const ChMainFrame* pCurr ) :
							m_pCurr( pCurr ),
							m_pNextFrame( 0 )
				{
				}

		virtual bool Visit( chparam key, const ChMainFrame* pFrame );

		inline const ChMainFrame* GetFrame() const { return m_pNextFrame; }

	private :
		const ChMainFrame*	m_pCurr;
		const ChMainFrame*	m_pNextFrame;
};


bool ChVisitGetNext::Visit( chparam key, const ChMainFrame* pFrame  )
{
	if (  pFrame != m_pCurr )
	{
		m_pNextFrame = pFrame;
		return false;
	}
	return true;
}


/*----------------------------------------------------------------------------
	ChEnumFrames class
----------------------------------------------------------------------------*/

class ChEnumFrames : public	ChPtrSplayVisitor2<ChMainFrame>
{
	public:
		ChEnumFrames( ChPuebloFrameVisitor* const pEnum ) :
						m_pEnum( pEnum )
						 {
						 }

		virtual bool Visit( chparam key, const ChMainFrame* pFrame );

	private :
		ChPuebloFrameVisitor* const m_pEnum;
};


bool ChEnumFrames::Visit( chparam key, const ChMainFrame* pFrame )
{
	return m_pEnum->Visit( (ChMainFrame* const)pFrame );
}


/*----------------------------------------------------------------------------
	ChVisitDeleteAll class
----------------------------------------------------------------------------*/

class ChVisitDeleteAll : public	ChPtrSplayVisitor2<ChMainFrame>
{
	public:
		ChVisitDeleteAll( ChMainFrame* pMainWnd ) :
						m_pMainFrame( pMainWnd )
				{
				}

		virtual bool Visit( chparam key, const ChMainFrame* pFrame );

	private:
		ChMainFrame*	m_pMainFrame;
};


bool ChVisitDeleteAll::Visit( chparam key, const ChMainFrame* pFrame  )
{
	if (m_pMainFrame != pFrame)
	{
		((ChMainFrame* const)pFrame)->SendMessage( WM_COMMAND, ID_FILE_CLOSE );
	}
	return true;
}


class ChFrameTickVisitor : public ChPuebloFrameVisitor 
{
	public:
		ChFrameTickVisitor( time_t timeCurr ) : m_timeCurr( timeCurr ) {}

		virtual bool Visit( const ChMainFrame* pFrame );

	protected:
		time_t		m_timeCurr;
};


bool ChFrameTickVisitor::Visit( const ChMainFrame* pFrame )
{
											/* Unfortunately, we need to cast
												the pointer here.  Not
												perfectly safe, but it should
												be okay for ticks. */

	((ChMainFrame*)pFrame)->OnSecondTick( m_timeCurr );

	return true;
}


/*----------------------------------------------------------------------------
	ChApp::OnUpdateNewPuebloFrame

		Message handler for New Pueblo frame.
----------------------------------------------------------------------------*/


void ChApp::OnUpdateNewPuebloFrame(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if ( m_pCurrFrame &&  
				((ChMainFrame*)m_pCurrFrame)->GetPuebloCore()->
				GetClientMode() == 	ChClientCore::modeDisabled)
	{
	 	pCmdUI->Enable( false );
	}
	
}

/*----------------------------------------------------------------------------
	ChApp::OnNewPuebloFrame

		Message handler for New Pueblo frame.
----------------------------------------------------------------------------*/

void ChApp::OnNewPuebloFrame()
{
	CreateNewFrame();
}

/*----------------------------------------------------------------------------
	ChApp::AddToFrameList

		Add a new frame to the list.
----------------------------------------------------------------------------*/


void ChApp::AddToFrameList( ChMainFrame* pFrame )
{
	if (!m_pFrameList.Find( (chparam)pFrame ))
	{
		m_pFrameList.Insert( (chparam)pFrame, pFrame );
	}
}


/*----------------------------------------------------------------------------
	ChApp::RemoveFromFrameList

		Removes a frame from the list.
----------------------------------------------------------------------------*/

void ChApp::RemoveFromFrameList( ChMainFrame* pFrame )
{
	if (!m_boolInDelete)
	{
		if (m_pFrameList.Find( (chparam)pFrame ))
		{
			m_pFrameList.Delete( (chparam)pFrame );
		}
	}
}


/*----------------------------------------------------------------------------
	ChApp::CreateNewFrame

		Create a new frame window with the specified arguments and label.
----------------------------------------------------------------------------*/

void ChApp::CreateNewFrame( const ChString& strArgs, const ChString& strLabel )
{
	#if ( _MFC_VER > 0x0400	 )
	POSITION		pos = 	GetFirstDocTemplatePosition( );

	#else
	POSITION		pos = m_templateList.GetHeadPosition();
	#endif

	CDocTemplate*	pTemplate;
	ChMainFrame*	pFrame;

	ASSERT( pos );

	#if ( _MFC_VER > 0x0400	 )
	pTemplate =		GetNextDocTemplate(  pos );
	#else					
	pTemplate = (CDocTemplate*)m_templateList.GetNext( pos );
	#endif

	pFrame = (ChMainFrame*)pTemplate->CreateNewFrame( 0, 0 );
	ASSERT( pFrame );
											// Set the label
	pFrame->SetLabel( strLabel );
											// Perform the initial update

	pTemplate->InitialUpdateFrame( pFrame, 0, true );

	pFrame->GetPuebloCore( )->StartPueblo( strArgs );

}


const ChMainFrame* ChApp::FindFrame( const char* pstrFrameName )
{
	ChVisitFind find( pstrFrameName ? pstrFrameName : "" );

	m_pFrameList.Infix( find );

	return find.GetFrame();
}


/*----------------------------------------------------------------------------
	ChApp::GetNextFrame

		Given a frame returns the next frame in the list, NULL if there is no
		other frame.
----------------------------------------------------------------------------*/

const ChMainFrame* ChApp::GetNextFrame( ChMainFrame* pCurr )
{
	if (!m_boolInDelete)
	{
		ChVisitGetNext visitNext( pCurr );

		m_pFrameList.Infix( visitNext );

		return visitNext.GetFrame();
	}

	return 0;
}


/*----------------------------------------------------------------------------
	ChApp::EnumerateFrames

		Enumerate all frames.
----------------------------------------------------------------------------*/

void ChApp::EnumerateFrames( ChPuebloFrameVisitor* pEnumFrames )
{
	ChEnumFrames	enumAll( pEnumFrames );

	m_pFrameList.Infix( enumAll );
}


/*----------------------------------------------------------------------------
	ChApp::OnSecondTick

		Called at least once per second.  Calls OnTick in each module.
----------------------------------------------------------------------------*/

void ChApp::OnSecondTick( time_t timeCurr )
{
	ChFrameTickVisitor		tickVisitor( timeCurr );

	EnumerateFrames( &tickVisitor );
}


/*----------------------------------------------------------------------------
	ChApp::OnAppExit

		Message handle for exit command
----------------------------------------------------------------------------*/

void ChApp::OnAppExit()
{
	// Close all Pueblo frames
	ChVisitDeleteAll deleteAll( (ChMainFrame*)CWinThread::m_pMainWnd );

	m_boolInDelete = true;

	m_pFrameList.Infix( deleteAll );

	m_pFrameList.Erase();

	m_boolInDelete = false;

	CWinApp::OnAppExit();
}


/*----------------------------------------------------------------------------
	ChApp::HandleOutdatedClient

		Deal with prompting the user about their outdated client.
		To be called on shutdown, but will only do something if the
		user has requested to be notified later.
----------------------------------------------------------------------------*/

void ChApp::HandleOutdatedClient()
{
	if (m_clientOutdated && (m_pMainWnd != NULL)) {
		ChString strTitle;
		ChString strMessage;
		
		LOADSTRING(IDS_TITLE_OUTOFDATE, strTitle);
		LOADSTRING(IDS_MESSAGE_OUTOFDATE, strMessage);
		if(::MessageBox(NULL, strMessage, strTitle, MB_YESNO | MB_ICONQUESTION) == IDYES) {
			ChString strFormat, strURL;
			LOADSTRING( IDS_UPDATE_URL, strFormat );
			ChString strClientVersion = ChCore::GetClientInfo()->
									GetClientVersion().Format( ChVersion::formatShort );
			strURL.Format( strFormat, LPCSTR(strClientVersion) );
			((ChMainFrame *)m_pMainWnd)->GetPuebloCore()->DisplayWebPage(strURL, ChCore::browserExternal);
		}
	}
}
