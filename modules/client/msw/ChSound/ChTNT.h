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

	Header file for the ChTNT object, which manages the interface between
	TNT and the Sound Module.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHTNT_H )
#define _CHTNT_H

#if defined( CH_USE_VOXWARE )

#define VOXWARE_WIN32
#include <tnt.h>


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define SESSION_LIMIT		5


/*----------------------------------------------------------------------------
	ChTNTSession class
----------------------------------------------------------------------------*/

class ChTNTSession
{
	public:
		ChTNTSession();
		~ChTNTSession();

	public:
		inline const ChString& GetId() { return m_strCallId; }
		inline TNT_SESSION_HANDLE GetSessionHdl() { return m_hSession; }
		inline chflag32 GetOptions() { return m_flOptions; }

		inline void Set( TNT_SESSION_HANDLE hSession,
							const ChString& strCallId,
							chflag32 flOptions )
					{
						m_strCallId = strCallId;
						m_hSession = hSession;
						m_flOptions = flOptions;
					}
		inline void SetSessionHdl( TNT_SESSION_HANDLE hSession )
					{
						m_hSession = hSession;
					}

	protected:
		ChString						m_strCallId;
		TNT_SESSION_HANDLE			m_hSession;
		chflag32					m_flOptions;
};


/*----------------------------------------------------------------------------
	ChTNTSessionMgr class
----------------------------------------------------------------------------*/

class ChTNTSessionMgr
{
	public:
		ChTNTSessionMgr();
		~ChTNTSessionMgr();

	public:
		inline bool AllSessionsInUse()
						{
							return (SESSION_LIMIT <= GetSessionCount());
						}

		inline int GetSessionCount() { return m_iSessions; }

		bool Set( const ChString& strCallId, TNT_SESSION_HANDLE hSession,
					chflag32 flOptions );

		TNT_SESSION_HANDLE GetSessionHandle( const ChString& strCallId,
												chflag32* pflOptions = 0 );
		bool Hangup( TNT_HANDLE hTNT, const ChString& strCallId );
		bool ClearSession( const ChString& strCallId );
		bool ClearSessionHdl( TNT_SESSION_HANDLE hSession );

		ChTNTSession* Find( const ChString& strCallId )
						{
							return Find( strCallId, 0 );
						}
		ChTNTSession* Find( TNT_SESSION_HANDLE hSession )
						{
							return Find( hSession, 0 );
						}

	protected:
		inline bool IsInUse( int iSession )
						{
							return m_boolSessionInUse[iSession];
						}
		inline void SetInUse( int iSession, bool boolInUse = true )
						{
							if (IsInUse( iSession ) && !boolInUse)
							{
								++m_iSessions;
							}
							else if (!IsInUse( iSession ) && boolInUse)
							{
								--m_iSessions;
							}

							m_boolSessionInUse[iSession] = boolInUse;
						}
		inline ChTNTSession* GetSession( int iSession )
						{
							return &m_sessionList[iSession];
						}

		ChTNTSession* Find( const ChString& strCallId, int* piLoc );
		ChTNTSession* Find( TNT_SESSION_HANDLE hSession, int* piLoc );

		bool Hangup( TNT_HANDLE hTNT, TNT_SESSION_HANDLE hSession );

	protected:
		int				m_iSessions;

		ChTNTSession	m_sessionList[SESSION_LIMIT];
		bool			m_boolSessionInUse[SESSION_LIMIT];
};


/*----------------------------------------------------------------------------
	ChTNT class
----------------------------------------------------------------------------*/

class ChSpeechStatus;

class ChTNT
{
	public:
		ChTNT( ChSpeechStatus* pSpeechStatus );
		~ChTNT();

		static chuint16 GetTNTPort() { return m_suTNTPort; }

		static ChTNT* GetTNT() { return m_this; }

		static bool MakeCall( const ChString& strCallId, const ChString& strHost,
								chuint16 suPort, chflag32 flOptions,
								chflag32 flRemoteOptions );
		static bool Hangup( const ChString& strCallId );

		void ForceTalk( bool boolTalk );

		static void PlayVoxFile( const ChString& strFilepath );

		static void SetMikeSensitivity( chuint32 luValue );
		static void SetMikeVolume( chuint32 luValue );

		static void UpdatePrefs();

		static ChSpeechStatus* GetStatus() { return m_pStatus; }

	protected:
		static TNT_HANDLE GetTNTHandle() { return m_hTNT; }
		static const ChString& GetTNTLocalHost() { return m_strTNTLocalHost; }
		static chuint32 GetMikeVolume() { return m_luMikeVolume; }
		static chuint32 GetMikeSensitivity() { return m_luMikeSensitivity; }
		static bool IsRejectingCalls() { return m_boolRejectCalls; }

		static void SetTNTPort( chuint16 suPort )
						{
							m_suTNTPort = suPort;
						}
		static void SetTNTLocalHost( const char* pstrLocalHost )
						{
							m_strTNTLocalHost = pstrLocalHost;
						}

		static TNT_RETCODE TNT_FAR TNT_PASCAL TNT_LOADDS
			NotifyProc( TNT_HANDLE hTNT, unsigned short wMessage,
						TNT_TRANSACTION_HANDLE hTransaction,
	    				unsigned long dwParam1, unsigned long dwParam2,
	    				TNT_USER_HANDLE hUser );

	protected:
		static ChSpeechStatus*		m_pStatus;
		static ChTNT*				m_this;
		static int					m_iUsage;

		static TNT_HANDLE			m_hTNT;
		static chuint16				m_suTNTPort;
		static ChString				m_strTNTLocalHost;

		static ChTNTSessionMgr		m_tntSessions;

		static TNT_USER_INFO		m_userInfo;

											// User preferences...

		static chuint32				m_luMikeVolume;
		static chuint32				m_luMikeSensitivity;
		static bool					m_boolRejectCalls;
};


#endif	// defined( CH_USE_VOXWARE )
#endif	// !defined( _CHTNT_H )

// $Log$
