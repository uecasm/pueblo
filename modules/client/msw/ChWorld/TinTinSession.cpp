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

	TinTin class miscellaneous methods.  Originally modified from TinTin++,
	(T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t, originally coded by
	Peter Unold 1992.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"
#if !defined(CH_PUEBLO_PLUGIN)
#include "resource.h"
#else
#include "vwrres.h"
#endif

#include <ChCore.h>

#include "TinTin.h"
#include "World.h"


/*----------------------------------------------------------------------------
	ChFrameListVisitor class
----------------------------------------------------------------------------*/

class ChFrameListVisitor: public ChFrameVisitor
{
	public:
		ChFrameListVisitor( const ChString& strMatch = "" ) :
					m_strMatch( strMatch ) {}
		virtual ~ChFrameListVisitor() {}

		inline const ChString& GetOutput() const { return m_strOutput; }

		virtual bool Visit( const ChString& strFrameName, ChCore* pCore );

	protected:
		ChString			m_strMatch;
		ChString			m_strOutput;
};


bool ChFrameListVisitor::Visit( const ChString& strFrameName,
									ChCore* pCore )
{
	ChWorldMainInfo*	pWorldMainInfo;

	if (pWorldMainInfo =
			(ChWorldMainInfo*)pCore->GetMainInfo( CH_MODULE_WORLD ))
	{
		if (m_strMatch.IsEmpty() || ::Match( m_strMatch, strFrameName ))
		{
			ChString		strTempName( strFrameName );
			ChString		strLine;
			ChString		strFormat;
			ChString		strConnection;

			if (pWorldMainInfo->IsConnected())
			{
				ChWorldInfo*	pWorldInfo = pWorldMainInfo->GetWorldInfo();

				ASSERT( 0 != pWorldInfo );
				strConnection.Format( "%s %d",
										(const char*)pWorldInfo->GetHost(),
										(int)pWorldInfo->GetPort() );
			}
			else
			{
				strConnection = "<not connected>";
			}

			if (strTempName.IsEmpty())
			{
				LOADSTRING( IDS_TINTIN_SESSION_LIST_NAMELESS, strTempName );
			}

			LOADSTRING( IDS_TINTIN_SESSION_LIST_ITEM, strFormat );
			strLine.Format( strFormat, (const char*)strTempName.Left( 20 ),
											(const char*)strConnection );
			m_strOutput += strLine + "\n";
		}
	}

	return true;
}


/*----------------------------------------------------------------------------
	ChFrameListVisitor class
----------------------------------------------------------------------------*/

class ChDoAllVisitor: public ChFrameVisitor
{
	public:
		ChDoAllVisitor( const ChString& strCommand ) :
					m_strCommand( strCommand ),
					m_iCount( 0 ) {}

		inline const ChString& GetCommand() const { return m_strCommand; }
		inline int GetCount() const { return m_iCount; }

		virtual bool Visit( const ChString& strFrameName, ChCore* pCore );

	protected:
		ChString			m_strCommand;
		int				m_iCount;
};


bool ChDoAllVisitor::Visit( const ChString& strFrameName,
								ChCore* pCore )
{
	ChWorldMainInfo*	pWorldMainInfo;

	if (pWorldMainInfo =
			(ChWorldMainInfo*)pCore->GetMainInfo( CH_MODULE_WORLD ))
	{
		if (pWorldMainInfo->IsConnected())
		{
			pWorldMainInfo->GetTinTin()->ParseInput( GetCommand() );
			m_iCount++;
		}
	}

	return true;
}


/*----------------------------------------------------------------------------
	TinTin class
----------------------------------------------------------------------------*/

void TinTin::DoAll( const ChString& strArgs )
{
	ChDoAllVisitor	doAllVisitor( strArgs );

	GetMainInfo()->GetCore()->EnumerateFrames( doAllVisitor );

	if (0 == doAllVisitor.GetCount())
	{
		ChString		strMessage;

		LOADSTRING( IDS_TINTIN_ALL_NO_SESSIONS, strMessage );
		ErrMessage( strMessage );
	}
}


void TinTin::DoName( const ChString& strArgs )
{
	const char*		pstrArgs = strArgs;
	ChString			strName;
	ChString			strFormat;
	ChString			strMessage;
	bool			boolSuccess = false;

	GetArgInBraces( pstrArgs, strName, true );

	if (strName.IsEmpty())
	{
		LOADSTRING( IDS_TINTIN_NAME_PARAM_ERR, strMessage );
		ErrMessage( strMessage );
	}
	else if (GetMainInfo()->GetCore()->GetFrameName() == strName)
	{
		boolSuccess = true;
	}
	else if (GetMainInfo()->GetCore()->GetMainInfo( CH_MODULE_WORLD, strName ))
	{
		LOADSTRING( IDS_TINTIN_SESSION_NAME_IN_USE, strFormat );
		strMessage.Format( strFormat, (const char*)strName );
		ErrMessage( strMessage );
	}
	else
	{
		GetMainInfo()->GetCore()->SetFrameName( strName );
		boolSuccess = true;
	}

	if (boolSuccess)
	{
		LOADSTRING( IDS_TINTIN_NAME_OK, strFormat );
		strMessage.Format( strFormat, (const char*)strName );
		Message( strMessage );
	}
}


void TinTin::DoSession( const ChString& strArgs )
{
	const char*		pstrArgs = strArgs;
	ChString			strLeft;
	ChString			strRight;
	ChString			strMessage;

	pstrArgs = GetArgInBraces( pstrArgs, strLeft, false );
	pstrArgs = GetArgInBraces( pstrArgs, strRight, true );

	if (strRight.IsEmpty())
	{
		ChFrameListVisitor	listVisitor( strLeft );

		GetMainInfo()->GetCore()->EnumerateFrames( listVisitor );

		if (listVisitor.GetOutput().IsEmpty())
		{
			LOADSTRING( IDS_TINTIN_SESSION_LIST_EMPTY, strMessage );
			Message( strMessage );
		}
		else
		{
			LOADSTRING( IDS_TINTIN_SESSION_LIST_HDR, strMessage );
			Message( strMessage );
			Message( listVisitor.GetOutput(), true );
		}
	}
	else
	{
		if (GetMainInfo()->GetCore()->GetMainInfo( CH_MODULE_WORLD, strLeft ))
		{
			ChString		strFormat;

			LOADSTRING( IDS_TINTIN_SESSION_NAME_IN_USE, strFormat );
			strMessage.Format( strFormat, (const char*)strLeft );
			Message( strMessage );
		}
		else
		{
			CreateSession( strLeft, strRight );
		}
	}
}


void TinTin::CreateSession( const ChString& strLabel,
							const ChString& strConnInfo )
{
	const char*	pstrConn;
	ChString		strTemp;

	pstrConn = SpaceOut( strConnInfo );
	strTemp = pstrConn;
	if (!strTemp.IsEmpty())
	{
		ChString	strHost;

		pstrConn = strTemp;
		while (*pstrConn && !isspace( *pstrConn ))
		{
			pstrConn++;
		}

		strHost = strTemp.Left( pstrConn - (const char*)strTemp );
		strTemp = strTemp.Mid( strHost.GetLength() );

		if (strTemp.GetLength())
		{
			pstrConn = SpaceOut( strTemp );

			if (isdigit( *pstrConn ))
			{
				int		iPort;
				ChString	strCommandLine;

				iPort = atoi( pstrConn );

				strCommandLine.Format( "%s %d", (const char*)strHost, iPort );

				GetMainInfo()->GetCore()->NewFrameWnd( strCommandLine,
														strLabel );
			}
			else
			{
				ChString		strMessage;

				LOADSTRING( IDS_TINTIN_SESSION_PORT_ERR, strMessage );
				Message( strMessage );
			}
		}
		else
		{
			ChString		strMessage;

			LOADSTRING( IDS_TINTIN_SESSION_PARAM_ERR, strMessage );
			Message( strMessage );
		}
	}
	else
	{
		ChString		strMessage;

		LOADSTRING( IDS_TINTIN_SESSION_PARAM_ERR, strMessage );
		Message( strMessage );
	}
}

// $Log$
