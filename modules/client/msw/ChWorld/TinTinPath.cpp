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
#include <ChDlg.h>

#include "headers.h"
#include "World.h"
#include "ChTextInput.h"
#include "ChTextOutput.h"

#include "TinTin.h"
#include "TinTinInfo.h"


/*----------------------------------------------------------------------------
	TinTin class
----------------------------------------------------------------------------*/

void TinTin::CheckInsertPath( const ChString& strCommand )
{
	TinTinListNode*	pNode = GetListPath()->GetTop();

	if (!m_boolTrackPaths)
	{
		return;
	}

	if (pNode = GetListPathDirs()->Search( strCommand ))
	{
		if (GetListPath()->GetCount() >= MAX_PATH_LENGTH)
		{
											// Delete the top node

			GetListPath()->DeleteNode( GetListPath()->GetTop() );
		}

		GetListPath()->AddNode( pNode->GetLeft(), pNode->GetRight(), "0" );
	}
}


/*----------------------------------------------------------------------------
	TinTin class protected methods
----------------------------------------------------------------------------*/

void TinTin::DoMap( const ChString& strArgs )
{
	ChString		strDirection;

	GetArgInBraces( strArgs, strDirection, true );
	CheckInsertPath( strDirection );
}


void TinTin::DoMark()
{
	ChString		strMessage;

	GetListPath()->Empty();

	LOADSTRING( IDS_TINTIN_MARK, strMessage );
	Message( strMessage );
}


void TinTin::DoPath()
{
	TinTinListNode*	pNode = GetListPath()->GetTop();

	if (pNode)
	{
		ChString			strMyPath;

		LOADSTRING( IDS_TINTIN_PATH_HDR, strMyPath );

		do {
			int		iDirectionLen = (pNode->GetLeft()).GetLength();

			if (iDirectionLen + strMyPath.GetLength() > 75)
			{
				Message( strMyPath );

				LOADSTRING( IDS_TINTIN_PATH_HDR, strMyPath );
			}

			strMyPath += pNode->GetLeft();
			strMyPath += " ";

			pNode = pNode->GetNext();

		} while (pNode);

		Message( strMyPath );
	}
	else
	{
		ChString		strMessage;

		LOADSTRING( IDS_TINTIN_PATH_EMPTY, strMessage );
		ErrMessage( strMessage );
	}
}


void TinTin::DoPathdir( const ChString& strArgs )
{
	TinTinListNode*	pNode = GetListPathDirs()->GetTop();
	const char*		pstrArgs = strArgs;
	ChString			strLeft;
	ChString			strRight;

	pstrArgs = GetArgInBraces( pstrArgs, strLeft, false );
	pstrArgs = GetArgInBraces( pstrArgs, strRight, true );

	if (strLeft.IsEmpty())
	{
		if (GetListPathDirs()->GetTop())
		{
			ChString		strMessage;

			LOADSTRING( IDS_TINTIN_PATHDIR_LIST_HDR, strMessage );
			Message( strMessage );

			GetListPathDirs()->ShowList( this );
		}
		else
		{
			ChString		strMessage;

			LOADSTRING( IDS_TINTIN_PATHDIR_LIST_EMPTY, strMessage );
			ErrMessage( strMessage );
		}
	}
	else if (!strLeft.IsEmpty() && strRight.IsEmpty())
	{
		TinTinListNode*		pNode;

		if (pNode = GetListPathDirs()->SearchWithWildchars( strLeft ))
		{
			do {
				GetListPathDirs()->ShowNode( this, pNode );
				pNode = pNode->GetNext();

			} while (pNode = GetListPathDirs()->SearchWithWildchars( strLeft,
																	pNode ));
		}
		else if (IsDisplayingMsg( msgIndexPathdir ))
		{
			ChString		strFormat;
			ChString		strMessage;

			LOADSTRING( IDS_TINTIN_NO_MATCHES, strFormat );
			strMessage.Format( strFormat, (const char*)strLeft );
			Message( strMessage );
		}
	}
	else
	{
		TinTinListNode*		pNode;

		if (pNode = GetListPathDirs()->Search( strLeft ))
		{
			GetListPathDirs()->DeleteNode( pNode );
		}

		GetListPathDirs()->InsertNode( strLeft, strRight, "0" );

		if (IsDisplayingMsg( msgIndexPathdir ))
		{
			ChString		strFormat;
			ChString		strMessage;

			LOADSTRING( IDS_TINTIN_NEW_PATHDIR, strFormat );
			strMessage.Format( strFormat, (const char*)strLeft,
											(const char*)strRight );
			Message( strMessage );
		}
		
		m_iPathdirCounter++;
	}
}


void TinTin::DoReturn()
{
	TinTinListNode*	pNode = GetListPath()->GetTop();

	if (pNode)
	{
		ChString		strCommand;
		bool		boolOldTrackPaths;
											// Find the last node
		while (pNode->GetNext())
		{
			pNode = pNode->GetNext();
		}
											// Save the 'return' command
		strCommand = pNode->GetRight();
											/* Temporarily turn off path
												tracking */
		boolOldTrackPaths = m_boolTrackPaths;
		m_boolTrackPaths = false;
											// Do the opposite direction
		ParseInput( strCommand );

		m_boolTrackPaths = boolOldTrackPaths;

		GetListPath()->DeleteNode( pNode );
	}
	else
	{
		ChString		strMessage;

		LOADSTRING( IDS_TINTIN_PATH_EMPTY, strMessage );
		ErrMessage( strMessage );
	}
}


void TinTin::DoSavepath( const ChString& strArgs )
{
	ChString			strName;
	TinTinListNode*	pNode = GetListPath()->GetTop();

	GetArgInBraces( strArgs, strName, true );

	if (strName.IsEmpty())
	{
		ChString		strMessage;

		LOADSTRING( IDS_TINTIN_SAVEPATH_NO_NAME, strMessage );
		ErrMessage( strMessage );
	}
	else if (pNode == 0)
	{
		ChString		strMessage;

		LOADSTRING( IDS_TINTIN_SAVEPATH_EMPTY, strMessage );
		ErrMessage( strMessage );
	}
	else
	{
		ChString			strResult;
		ChString			strFormat;

		LOADSTRING( IDS_TINTIN_SAVEPATH_FMT, strFormat );

		strResult.Format( strFormat, GetTinTinChar(), (const char*)strName );
		strResult += " {";

		while (pNode)
		{
			strResult += pNode->GetLeft();
			strResult += ";";
			pNode = pNode->GetNext();
		}

		strResult = strResult.Left( strResult.GetLength() - 1 );
		strResult += "}";

		ParseInput( strResult );
	}
}

void TinTin::DoUnpath()
{
	TinTinListNode*	pNode = GetListPath()->GetTop();

	if (pNode)
	{
		ChString		strDirection;
		ChString		strFormat;
		ChString		strMessage;
											// Find the last node
		while (pNode->GetNext())
		{
			pNode = pNode->GetNext();
		}
											// Delete the node
		strDirection = pNode->GetLeft();
		GetListPath()->DeleteNode( pNode );

		LOADSTRING( IDS_TINTIN_UNPATH_OK, strFormat );
		strMessage.Format( strFormat, (const char*)strDirection );
		Message( strMessage );
	}
	else
	{
		ChString		strMessage;

		LOADSTRING( IDS_TINTIN_PATH_EMPTY, strMessage );
		ErrMessage( strMessage );
	}
}

// $Log$
