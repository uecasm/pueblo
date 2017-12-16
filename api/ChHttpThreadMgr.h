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

#if !defined( CH_HTTPTHREADMGR_H )
#define CH_HTTPTHREADMGR_H

#if	defined( CH_MSW ) && defined( CH_ARCH_32 )
#include <ChList.h>

class ChHttpThread;
class ChHttpThreadMgr;

typedef ChPtrList<ChHttpThread>	ChThreadList; 


class ChHttpThread
{
	public :
		enum tagThreadEvents { eventProcess = 0, eventTerminate, maxEvents };

		ChHttpThread( ChHttpThreadMgr* pMgr, bool boolCritical );
		~ChHttpThread();

		int		GetTotalThreadEvents()			{ return maxEvents; }
		HANDLE* GetHttpThreadEvents()			{ return m_httpThreadEvents; }
		bool	IsIdle()						{ return m_boolIdle; }			
		bool	IsWorking()						{ return m_boolWorking; }			
		bool	IsCritical()					{ return m_boolCritical; }
		bool	IsDead()						{ return m_boolDead; }
		ChHttpThreadMgr* GetThreadMgr()			{ return m_pThreadMgr; }

		void    TriggerThreadTerminate()		
						{ 
							::SetEvent( m_httpThreadEvents[eventTerminate] ); 
						}

		void    TriggerProcessRequest()			{ ::SetEvent( m_httpThreadEvents[eventProcess] ); }


		void    SetWorking( bool boolState )	{  m_boolWorking = boolState; }
		void    SetDead( bool boolDead )		{  m_boolDead = boolDead; }
		void    SetIdle( bool boolIdle )		{  m_boolIdle = boolIdle; }

		void	LockStream()
					{
						::EnterCriticalSection( &m_httpThreadSync );
					}
		void	UnlockStream()
					{
						::LeaveCriticalSection( &m_httpThreadSync );
					}


	private:
		bool					m_boolCritical;
		bool					m_boolIdle;
		bool					m_boolDead;
		bool					m_boolWorking;
		HANDLE					m_httpThreadEvents[maxEvents];	
		CRITICAL_SECTION		m_httpThreadSync;
		ChHttpThreadMgr*		m_pThreadMgr;
		
};

class ChHttpThreadMgr
{
	public :
		enum tagConst { maxIdle = 2 };
		enum tagEvents { requestEvent = 0, workeridleEvent, workerDieEvent, shutdownEvent, maxEvents };
		ChHttpThreadMgr();
		~ChHttpThreadMgr();

		HANDLE*	GetEvents()							{ return m_hEvents; }	
		int		GetTotalEvents()					{ return maxEvents; }	
		HANDLE	GetShutdownAckEvent( )				{ return m_hTerminateAckEvent; }	

		bool	IsRequestEvent( chint32 lEvent )	{ return lEvent == requestEvent; }	
		bool	IsWorkerIdleEvent( chint32 lEvent )	{ return lEvent == workeridleEvent; }	
		bool	IsWorkerDieEvent( chint32 lEvent  )	{ return lEvent == workerDieEvent; }
		bool	IsShutdownEvent( chint32 lEvent )	{ return lEvent == shutdownEvent; }

		// To the main thread when the request thread terminates
		void    TriggerRequestEvent()		{ ::SetEvent( m_hEvents[requestEvent] ); }
		void    TriggerIdle()				{ ::SetEvent( m_hEvents[workeridleEvent] ); }
		void    TriggerTerminate()			{ ::SetEvent( m_hEvents[workerDieEvent] ); }
		void    TriggerShutdown()			{ ::SetEvent( m_hEvents[shutdownEvent] ); }

		void    TriggerAckEvent()			{ ::SetEvent( m_hTerminateAckEvent ); }

		ChThreadList* GetThreadList()					{ return 	&m_httpThreadList; }

		void 	ProcessRequestEvent();
		void 	ProcessIdleEvent();
		bool 	ProcessWorkerDieEvent();
		bool 	ProcessShutdownEvent();

	private:
		bool				m_boolShutdown;
		HANDLE				m_hEvents[maxEvents];	
		HANDLE				m_hTerminateAckEvent;	
		ChThreadList		m_httpThreadList; 
};

#if	defined(CH_MSW ) && defined( WIN32 )
UINT HTTPProcessDownload( LPVOID pData );

UINT HTTPRequestThread( LPVOID pData );
#endif


#endif //#if	defined( CH_MSW ) && defined( CH_ARCH_32 )

#endif //#if !defined( CH_HTTPTHREADMGR_H )
