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

	TinTin class main object.  Originally modified from TinTin++,
	(T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t, originally coded by
	Peter Unold 1992.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include "TinTin.h"
#include "TinTinSettings.h"


/*----------------------------------------------------------------------------
	TinTinSettings class
----------------------------------------------------------------------------*/

TinTinSettings::TinTinSettings()
{
	Initialize();
	Reset();
}


TinTinSettings::TinTinSettings( TinTinSettings& src )
{
	Initialize();
	*this = src;
}


TinTinSettings::~TinTinSettings()
{
	if (m_pListActions)
	{
		delete m_pListActions;
		m_pListActions = 0;
	}

	if (m_pListAliases)
	{
		delete m_pListAliases;
		m_pListAliases = 0;
	}

	if (m_pListHighlights)
	{
		delete m_pListHighlights;
		m_pListHighlights = 0;
	}

	if (m_pListPathDirs)
	{
		delete m_pListPathDirs;
		m_pListPathDirs = 0;
	}

	if (m_pListSubs)
	{
		delete m_pListSubs;
		m_pListSubs = 0;
	}

	if (m_pListAntiSubs)
	{
		delete m_pListAntiSubs;
		m_pListAntiSubs = 0;
	}

	if (m_pListVars)
	{
		delete m_pListVars;
		m_pListVars = 0;
	}
}


TinTinSettings& TinTinSettings::operator=( TinTinSettings& src )
{
	int		iLoop;

	*m_pListActions = *(src.GetListActions());
	*m_pListAliases = *(src.GetListAliases());
	*m_pListVars = *(src.GetListVars());
	*m_pListHighlights = *(src.GetListHighlights());
	*m_pListSubs = *(src.GetListSubs());
	*m_pListAntiSubs = *(src.GetListAntiSubs());
	*m_pListPathDirs = *(src.GetListPathDirs());

	SetTinTinChar( src.GetTinTinChar() );

	for (iLoop = 0; iLoop < msgIndexLast; iLoop++)
	{
		SetDisplayingMsg( iLoop, src.IsDisplayingMsg( iLoop ) );
	}

	SetEcho( src.IsEcho() );
	SetIgnore( src.IsIgnore() );
	SetPresub( src.IsUsingPresubs() );
	SetSpeedwalk( src.IsSpeedwalk() );
	SetUsingSubs( src.IsUsingSubs() );
	SetVerbatim( src.IsVerbatim() );
	SetTicking( src.IsTicking() );
	SetTickSize( src.GetTickSize() );
	SetTickKey( src.GetTickKey() );

	return *this;
}


void TinTinSettings::Reset()
{
	SetTinTinChar( DEF_TINTIN_CHAR );
	SetEcho( false );
	SetIgnore( false );
	SetUsingSubs( true );
	SetPresub( true );
	SetSpeedwalk( true );
	SetVerbatim( true );

	SetTicking( false );
	SetTickSize( DEF_TICK_SIZE );
	SetTickKey( DEF_TICK_KEY );

	DoKillAll();

	m_boolDisplayMessages[msgIndexAliases] = true;
	m_boolDisplayMessages[msgIndexActions] = true;
	m_boolDisplayMessages[msgIndexSubstitutions] = true;
	m_boolDisplayMessages[msgIndexAntiSub] = true;
	m_boolDisplayMessages[msgIndexHilight] = true;
	m_boolDisplayMessages[msgIndexVariable] = true;
	m_boolDisplayMessages[msgIndexPathdir] = true;
}


/*----------------------------------------------------------------------------
	TinTinSettings::DoKillAll
				Empties the lists.
----------------------------------------------------------------------------*/

void TinTinSettings::DoKillAll()
{
	GetListAliases()->Empty();
	GetListActions()->Empty();
	GetListVars()->Empty();
	GetListHighlights()->Empty();
	GetListSubs()->Empty();
	GetListAntiSubs()->Empty();
	GetListPathDirs()->Empty();
}


/*----------------------------------------------------------------------------
	TinTinSettings protected methods
----------------------------------------------------------------------------*/

void TinTinSettings::Initialize()
{
											/* Create the object lists */

	m_pListActions = new TinTinList( PRIORITY );
	m_pListAliases = new TinTinList;
	m_pListHighlights = new TinTinList;
	m_pListPathDirs = new TinTinList;
	m_pListSubs = new TinTinList;
	m_pListAntiSubs = new TinTinList;
	m_pListVars = new TinTinList;
}

// $Log$
