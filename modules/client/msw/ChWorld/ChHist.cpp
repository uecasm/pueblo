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

	Implementation for the ChTextInputBar class.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"
#include "ChHist.h"
#include "MemDebug.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif


/*----------------------------------------------------------------------------
	ChHistory class
----------------------------------------------------------------------------*/
	
ChHistory::ChHistory( chint16 sMax ) :
				m_sMax( sMax ),
				m_curr( 0 ),
				m_state( empty )
{
}


ChHistory::~ChHistory()
{
	while (!m_historyList.IsEmpty())
	{
		RemoveHead();
	}
}


/*----------------------------------------------------------------------------
	ChHistory public methods
----------------------------------------------------------------------------*/
	
void ChHistory::Reset()
{
	if (m_historyList.GetCount())
	{
		m_curr = 0;
		m_state = notBrowsingHistory;
	}
	else
	{
		m_state = empty;
	}
}


void ChHistory::Add( const ChString& strLine )
{
	if ((0 == m_historyList.GetCount()) ||
		(strLine != m_historyList.GetTail()))
	{
											/* Add the data if it's not
												already the end of the
												history */
		m_historyList.AddTail( strLine );

		if (m_historyList.GetCount() > m_sMax)
		{
			RemoveHead();
		}
	}

	Reset();								// Reset the state
}


bool ChHistory::GetNext( ChString& strLine )
{
	bool	boolSuccess = true;

	switch( m_state )
	{
		case atTop:
		{									/* m_curr is meaningless */

			m_curr = m_historyList.GetHeadPosition();
			m_historyList.GetNext( m_curr );
			strLine = m_historyList.Get( m_curr );
			m_state = inMiddle;
			break;
		}

		case inMiddle:
		{									/* m_curr points to the last item
												displayed */
			m_historyList.GetNext( m_curr );
			if (0 == m_curr)
			{
				m_state = atEnd;
				boolSuccess = false;
			}
			else
			{
				strLine = m_historyList.Get( m_curr );
				m_state = inMiddle;
			}
			break;
		}

		case notBrowsingHistory:
		case onlyItem:
		case atEnd:
		case empty:
		{
			boolSuccess = false;
			break;
		}

		default:
		{
			TRACE( "Invalid state in ChHistory class during GetNext()" );
			ASSERT( true );
			break;
		}
	}

	return boolSuccess;
}


bool ChHistory::GetPrevious( ChString& strLine )
{
	bool	boolSuccess = true;

	switch( m_state )
	{
		case notBrowsingHistory:
		{									/* m_curr is meaningless */

			m_curr = m_historyList.GetTailPosition();
			strLine = m_historyList.Get( m_curr );

			if (m_historyList.GetCount() > 1)
			{
				m_state = inMiddle;
			}
			else
			{								// Dead end state
				m_state = onlyItem;
			}
			break;
		}

		case inMiddle:
		{									/* m_curr points to the last item
												that was displayed */

			m_historyList.GetPrev( m_curr );
			if (0 == m_curr)
			{
				m_state = atTop;
				boolSuccess = false;
			}
			else
			{
				strLine = m_historyList.Get( m_curr );
				m_state = inMiddle;
			}
			break;
		}

		case atEnd:
		{									/* m_curr is meaningless */

			m_curr = m_historyList.GetTailPosition();
			m_historyList.GetPrev( m_curr );
			strLine = m_historyList.Get( m_curr );
			m_state = inMiddle;
			break;
		}

		case onlyItem:
		case atTop:
		case empty:
		{
			boolSuccess = false;
			break;
		}

		default:
		{
			TRACE1( "Invalid state in ChHistory class during "
						"GetPrevious(): %d\n", (int)m_state );
			ASSERT( true );
			break;
		}
	}

	return boolSuccess;
}


bool ChHistory::GetExpansion( ChString& strText, ChPosition& startPos )
{
	bool		boolFound = false;

	if (0 == startPos)
	{
		startPos = m_historyList.GetTailPosition();
	}

	while (startPos && !boolFound)
	{
		ChString		strHistText = m_historyList.Get( startPos );

		if (0 == strHistText.Find( strText ))
		{
			strText = strHistText;
			boolFound = true;
		}

		m_historyList.GetPrev( startPos );
	}

	return boolFound;
}

// $Log$
// Revision 1.1.1.1  2003/02/03 18:53:08  uecasm
// Import of source tree as at version 2.53 release.
//
