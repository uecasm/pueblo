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

	This file contains the implementation of the HTTP thread manager

----------------------------------------------------------------------------*/

#include "headers.h"

#include <ChHtpCon.h>
#include <ChDb.h>
#include <SocketXX.h>
#include <ChSock.h>

#include "ChHTPriv.h"
#include "ChHttpThreadMgr.h"

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	ChHttpThreadMgr:: 
----------------------------------------------------------------------------*/
ChHttpThread::ChHttpThread( ChHttpThreadMgr* pMgr, bool boolCritical ) :
				m_pThreadMgr( pMgr ),
				m_boolCritical( boolCritical ),
				m_boolIdle( false ),
				m_boolDead( true )	
{
	::InitializeCriticalSection( &m_httpThreadSync );
	m_httpThreadEvents[eventProcess] =  ::CreateEvent( 0, false, false, 0 );
	ASSERT( m_httpThreadEvents[eventProcess]  );
	m_httpThreadEvents[eventTerminate] = ::CreateEvent( 0, false, false, 0 );
}

ChHttpThread::~ChHttpThread()
{
	::CloseHandle( m_httpThreadEvents[eventProcess] );
	::CloseHandle( m_httpThreadEvents[eventTerminate] );

	::DeleteCriticalSection( &m_httpThreadSync );
}

/*----------------------------------------------------------------------------
	ChHttpThreadMgr::AddCookie 
----------------------------------------------------------------------------*/

ChHttpThreadMgr::ChHttpThreadMgr() :
				m_boolShutdown( false )
{

									// Create all the events
	m_hEvents[requestEvent] = CreateEvent( 0, false, false, 0 );
	ASSERT( m_hEvents[requestEvent] );
	m_hEvents[workeridleEvent] = CreateEvent( 0, false, false, 0 );
	ASSERT( m_hEvents[workeridleEvent] );
	m_hEvents[workerDieEvent] = CreateEvent( 0, false, false, 0 );
	ASSERT( m_hEvents[workerDieEvent] );
	m_hEvents[shutdownEvent] = CreateEvent( 0, false, false, 0 );
	ASSERT( m_hEvents[shutdownEvent] );

	m_hTerminateAckEvent = CreateEvent( 0, false, false, 0 );
	ASSERT( m_hTerminateAckEvent );

}
ChHttpThreadMgr::~ChHttpThreadMgr()
{
	CloseHandle( m_hEvents[requestEvent] );
	CloseHandle( m_hEvents[workeridleEvent] );
	CloseHandle( m_hEvents[workerDieEvent] );
	CloseHandle( m_hEvents[shutdownEvent] );
	CloseHandle( m_hTerminateAckEvent );

}


void ChHttpThreadMgr::ProcessRequestEvent()
{
	TRACE( "Event: Process Request\n" ); 
	// See if we need to start any threads		   

	ChPosition posThread = m_httpThreadList.GetHeadPosition();
	ChHttpThread*		pThread = 0;

	while ( 0 != posThread )
	{
		pThread = m_httpThreadList.GetNext( posThread );

		if ( pThread->IsIdle() )
		{
			break;
		}
		else
		{
			pThread = 0;
		}

	}

	if ( pThread )
	{
		if ( pThread->IsDead() )
		{ // We need to start the thread
			pThread->SetDead( false );
			pThread->SetIdle( false );
			if ( !AfxBeginThread( HTTPProcessDownload, 
							(ptr)pThread ) )// THREAD_PRIORITY_BELOW_NORMAL ) )
			{
				pThread->SetIdle( true );
				pThread->SetDead( true );
			}
		}
		else
		{  // This is sleeping wake up the thread and assign work
			TRACE( "Wakeup worker thread\n" );
			pThread->SetIdle( false );
			pThread->TriggerProcessRequest();	
		}
	}
	else if ( m_httpThreadList.GetCount( ) < 	ChHTTPSocketConn::GetMaxConnections()  )
	{
		bool 		boolCritical = m_httpThreadList.GetCount( ) < maxIdle;
		pThread = new ChHttpThread( this, boolCritical );
		ASSERT( pThread );

		m_httpThreadList.AddTail( pThread );

		pThread->SetDead( false );
		pThread->SetIdle( false );

		if ( !AfxBeginThread( HTTPProcessDownload, 
						(ptr)pThread )) //, THREAD_PRIORITY_BELOW_NORMAL ) )
		{
			pThread->SetIdle( true );
			pThread->SetDead( true );
		}
	}

}

void ChHttpThreadMgr::ProcessIdleEvent()
{


	TRACE( "Event: Thread idle\n" );

	ChPosition posThread = m_httpThreadList.GetHeadPosition();
	ChHttpThread*		pThread;

   	// Set all non-working threads as idle
	while ( 0 != posThread )
	{
		pThread = m_httpThreadList.GetNext( posThread );
		// If the thread is not active set to idle mode so that request can use it
		if ( !pThread->IsWorking()  )
		{
			pThread->SetIdle( true );
		}
	}
	// if there is a pending request process it
 	if ( ChHTTPInfo::GetRequestQueue().GetCount() )
	{
		ProcessRequestEvent();
	}
	// if we have more than maxIdle threads then terminate all the excees threads
	if ( m_httpThreadList.GetCount( ) > maxIdle )
	{
		// remove excess threads
		ChPosition posThread = m_httpThreadList.GetHeadPosition();
		ChHttpThread*		pThread = 0;

		while ( 0 != posThread )
		{
			pThread = m_httpThreadList.GetNext( posThread );

			if ( pThread->IsIdle() && !pThread->IsCritical() && !pThread->IsDead()  )
			{
				pThread->TriggerThreadTerminate();
			}

		}

	}

}

bool ChHttpThreadMgr::ProcessWorkerDieEvent()
{
	TRACE( "Event: Worker thread terminates\n" );

	if ( m_boolShutdown )
	{

		ChPosition posThread = m_httpThreadList.GetHeadPosition();
		ChHttpThread*		pThread = 0;

		while ( 0 != posThread )
		{
			ChPosition posPrev = posThread;
			pThread = m_httpThreadList.GetNext( posThread );

			if ( pThread->IsDead()  )
			{
				m_httpThreadList.Remove( posPrev );

				delete pThread;
				posThread = m_httpThreadList.GetHeadPosition();
			}
		}

		if ( m_httpThreadList.GetCount() == 0 )
		{
		   	// Broad cast i am done    
			TriggerAckEvent();
			return true;
		}

	}

	return false;
}

bool ChHttpThreadMgr::ProcessShutdownEvent()
{

	TRACE( "Event: Shutdown HTTP request thread\n" );
	m_boolShutdown = true; // we are in shutdown mode now

	bool boolThreadsActive = false;

	if ( m_httpThreadList.GetCount( ) )
	{

		ChPosition posThread = m_httpThreadList.GetHeadPosition();
		ChHttpThread*		pThread = 0;

		while ( 0 != posThread )
		{
			ChPosition posPrev = posThread;
			pThread = m_httpThreadList.GetNext( posThread );

			if ( !pThread->IsDead()  )
			{
				boolThreadsActive = true;
				pThread->TriggerThreadTerminate();
			}
			else 
			{

				m_httpThreadList.Remove( posPrev );

				delete pThread;
				posThread = m_httpThreadList.GetHeadPosition();
			}

		}

	}
	// if no worker threads active then quit
	if ( !boolThreadsActive )
	{
		TriggerAckEvent();
	}

	return !boolThreadsActive;

}
