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

#include <ChReg.h>

#include "TinTin.h"
#include "MemDebug.h"


/*----------------------------------------------------------------------------
	TinTin class
----------------------------------------------------------------------------*/

const char		TinTin::m_cVerbatim = DEF_VERBATIM_CHAR;
TinTinSettings*	TinTin::m_pGlobalSettings = 0;
int				TinTin::m_iObjectCount = 0;
ChString			TinTin::m_strFalse;


TinTin::TinTin( ChWorldMainInfo* pMainInfo ) :
			m_pMainInfo( pMainInfo ),
			m_pSettings( 0 ),
			m_pActiveSettings( 0 ),
			m_iRecurseLevel( 0 ),
			m_boolUseGlobal( false ),
			m_boolOnline( true ),
			m_pListPath( 0 )
{
											/* Fill in this string just for
												the heck of it */
	m_strFalse.Format( "%d", false );
											// Create the path list
	m_pListPath = new TinTinList;

	if (0 == m_iObjectCount)
	{										// Create the global settings

		m_pGlobalSettings = new TinTinSettings;
		ASSERT( m_pGlobalSettings );
											// Read in the global settings
		ReadGlobalFile();
											// Init the random-number generator
		srand( (UINT)time( 0 ) );
	}
											// Increment the object count
	m_iObjectCount++;

	Reset();
}


TinTin::~TinTin()
{
											// Decrement the object count
	m_iObjectCount--;

	if (0 == m_iObjectCount)
	{										// Release the global settings
		delete m_pGlobalSettings;
		m_pGlobalSettings = 0;
	}
											// Release the local settings
	if (m_pSettings)
	{
		delete m_pSettings;
		m_pSettings = 0;
	}

	if (m_pListPath)
	{
		delete m_pListPath;
		m_pListPath = 0;
	}
}


void TinTin::SendToWorld( const ChString& strOutput )
{
}


void TinTin::Display( const ChString& strOutput, bool boolPreformatted, bool boolRenderHtml ) const
{
}


void TinTin::Reset()
{
	m_boolTrackPaths = true;

	m_iAliasCounter = 0;
	m_iActionCounter = 0;
	m_iSubCounter = 0;
	m_iAntiSubCounter = 0;
	m_iVarCounter = 0;
	m_iHighlightCounter = 0;
	m_iPathdirCounter = 0;

	SetMessages( true );

	if (m_pGlobalSettings)
	{										// Recreate the settings
		if (m_pSettings)
		{
			delete m_pSettings;
		}

		m_pSettings = new TinTinSettings( *m_pGlobalSettings );
	}
											/* Set the active settings to
												be the local settings */
	SetLocalSettings( true );
}


/*----------------------------------------------------------------------------
	TinTin protected methods
----------------------------------------------------------------------------*/

void TinTin::ReadGlobalFile()
{
	ChRegistry	reg( WORLD_PREFS_GROUP );
	ChString		strTinTinFile;
											// Make global settings active
	SetLocalSettings( false );

	reg.Read( WORLD_TINTIN_FILE, strTinTinFile, WORLD_TINTIN_FILE_DEF );
	if (!strTinTinFile.IsEmpty())
	{
		ReadInitFile( "{" + strTinTinFile + "}" );
	}
}

// $Log$
// Revision 1.2  2003/07/04 11:26:42  uecasm
// Update to 2.60 (see help file for details)
//
// Revision 1.1.1.1  2003/02/03 18:53:38  uecasm
// Import of source tree as at version 2.53 release.
//
