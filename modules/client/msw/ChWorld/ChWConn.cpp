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

     Ultra Enterprises:  Gavin Lambert
     
          Corrected and extended ANSI sequence parsing to properly
          support font styles.  The original code used the wrong
          value for "unbold" and didn't recognise "non-italic" and
          "de-underline".  In addition, the concept of "bold" in
          ANSI was changed to the common interpretation of low- vs.
          high-intensity colours rather than bold font-style.
					Support for MCCP added.  Fixed up Erase code parsing; it
					previously accepted ESC[2j but ESC[2J is in fact the
					correct code.  The lowercase j code is no longer supported
					(because it means something QUITE different).

------------------------------------------------------------------------------

	Defines the ChWorld module for the Pueblo system.  This module is
	used to connect to external virtual worlds.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <fstream>
#include <ctype.h>

#include <ChCore.h>
#include <ChReg.h>
#include <ChHtmWnd.h>

#include <ChSound.h>

#include "World.h"
#include "ChWConn.h"
#if !defined(CH_PUEBLO_PLUGIN)
#include "resource.h"
#else
#include "vwrres.h"
#endif

#include <zlib.h>
#include "MemDebug.h"

/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define MAX_LINE_LEN		4096			/* If a line is longer than 4k,
												then give up looking for a
												line ending */
#define BELL					0x07				// UE: ASCII BEL character (beep)
#define ESCAPE				0x1b
#define ANSI_START_1		ESCAPE
#define ANSI_START_2		'['
#define ANSI_SEPARATOR		';'
#define ANSI_MODE			'm'
#define ANSI_ERASE			'J'
											/* Telnet commands:
												(from RFC 854:  Telnet protocol
																spec.) */
#define TN_EOR				239				// End-Of-Record
#define TN_SE				240				// End of subnegotiation params
#define TN_NOP				241				// not used
#define TN_DATA_MARK		242				// not used
#define TN_BRK				243				// not used
#define TN_IP				244				// not used
#define TN_AO				245				// not used
#define TN_AYT				246				// not used
#define TN_EC				247				// not used
#define TN_EL				248				// not used
#define TN_GA				249				// Go Ahead
#define TN_SB				250				/* Indicates that what follows is
												subnegotiation of the indicated
                                 				option */
#define TN_WILL				251				// I offer to ~, or ack for DO
#define TN_WONT				252				// I will stop ~ing, or nack for DO
#define TN_DO				253				// Please do ~?, or ack for WILL
#define TN_DONT				254				// Stop ~ing!, or nack for WILL
#define TN_IAC				255				// Is A Command character

											// Telnet options:

#define TN_ECHO				1				// echo option
#define TN_SGA				3				// suppress GOAHEAD option
#define TN_STATUS			5				// not used
#define TN_TIMING_MARK		6				// not used
#define TN_TERMINAL_TYPE	24				// terminal type option
#define TN_EOR_OPT			25				// End Of Record option
#define TN_NAWS				31				// not used
#define TN_TSPEED			32				// not used
#define TN_LINEMODE			34				// not used

											// MCCP options:
#define TN_COMPRESS		85
#define TN_COMPRESS2	86

											// Other Telnet codes:
#define TN_IS				0
#define TN_SEND				1


/*----------------------------------------------------------------------------
	Types
----------------------------------------------------------------------------*/

typedef enum { betweenSequences, foundIAC, foundModifier } TelnetState;


/*----------------------------------------------------------------------------
	Static arrays
----------------------------------------------------------------------------*/

#if defined( DUMP_TELNET_CODES )

static const char*	pstrTelnetLabel[256];

#endif	// defined( DUMP_TELNET_CODES )

static const char*	pstrTerminalTypes[] = {
		"pueblo",
		"ansi",
		"network-virtual-terminal",
		"unknown",
		NULL
};


/*----------------------------------------------------------------------------
	ChBufferString class
----------------------------------------------------------------------------*/

ChBufferString::ChBufferString() :
					m_iBufferSize( initBufferStringSize ),
					m_iActualSize( 0 )
{
	m_pstrBuffer = m_string.GetBuffer( m_iBufferSize );
	m_pstrBufferEnd = m_pstrBuffer;
}


ChBufferString::ChBufferString( const ChString& strData )
{
	m_iActualSize = strData.GetLength();
	m_iBufferSize = m_iActualSize + growBufferStringSize;
	m_pstrBuffer = m_string.GetBuffer( m_iBufferSize );
	m_pstrBufferEnd = m_pstrBuffer + m_iActualSize;
}


ChBufferString::~ChBufferString()
{
}


ChBufferString& ChBufferString::operator +=( char cChar )
{
	if (m_iActualSize >= m_iBufferSize)
	{
		m_string.ReleaseBuffer( m_iActualSize );

											/* We need to reallocate the
												string to make room for the
												new data */

		m_iBufferSize = m_iActualSize + growBufferStringSize;

											// Get new pointers

		m_pstrBuffer = m_string.GetBuffer( m_iBufferSize );
		m_pstrBufferEnd = m_pstrBuffer + m_iActualSize;
	}

	*m_pstrBufferEnd = cChar;
	m_pstrBufferEnd++;
	*m_pstrBufferEnd = 0;
	m_iActualSize++;

	return *this;
}


ChBufferString& ChBufferString::operator +=( const char* pstrText )
{
	int		iLen = strlen( pstrText );

	return Append(pstrText, iLen);
}

ChBufferString& ChBufferString::Append( const char* pstrText, int iLen )
{
	if (m_iActualSize + iLen >= m_iBufferSize)
	{										/* We need to reallocate the
												string to make room for the
												new data */
		m_string.ReleaseBuffer( m_iActualSize );

											/* We need to reallocate the
												string to make room for the
												new data */

		m_iBufferSize = m_iActualSize + growBufferStringSize;
		while (m_iActualSize + iLen >= m_iBufferSize)
		{
			// adding a really big chunk o' text to this one
			m_iBufferSize += growBufferStringSize;
		}

											// Get new pointers

		m_pstrBuffer = m_string.GetBuffer( m_iBufferSize );
		m_pstrBufferEnd = m_pstrBuffer + m_iActualSize;
	}

	strncpy( m_pstrBufferEnd, pstrText, iLen );
	m_pstrBufferEnd += iLen;
	m_iActualSize += iLen;
	*m_pstrBufferEnd = 0;

	return *this;
}

ChBufferString& ChBufferString::DeleteFromStart( int iLen )
{
	if (m_iActualSize > iLen)
	{
		// Delete some characters from the start; first compact the data
		m_iActualSize -= iLen;
		memmove(m_pstrBuffer, m_pstrBuffer + iLen, m_iActualSize * sizeof(TCHAR));
		// Now try to shrink the buffer
		m_string.ReleaseBuffer( m_iActualSize );
		while (m_iBufferSize - growBufferStringSize > m_iActualSize)
			m_iBufferSize -= growBufferStringSize;
		// Adjust pointers accordingly
		m_pstrBuffer = m_string.GetBuffer( m_iBufferSize );
		m_pstrBufferEnd = m_pstrBuffer + m_iActualSize;
	}
	else
	{
		// Trivial case; we want to erase all (or more) data than we actually have
		Clear();
	}

	return *this;
}

ChBufferString& ChBufferString::Clear()
{
	m_string.ReleaseBuffer(0);
	m_iActualSize = 0;
	m_iBufferSize = initBufferStringSize;
	m_pstrBuffer = m_string.GetBuffer( m_iBufferSize );
	m_pstrBufferEnd = m_pstrBuffer;
	
	return *this;
}



/*----------------------------------------------------------------------------
	ChAnsiState class
----------------------------------------------------------------------------*/

// UE: This colour table has been extended a bit; in a future version I'm
//     planning to switch this all out to the Registry to allow user
//     configuration (mostly to let them select their own brightness levels).
const chuint32 ChAnsiState::colourTable[16] = {
	0x000000,		// black, low-intensity
	0xC00000,		// red
	0x00C000,		// green
	0xC0C000,		// yellow
	0x0000C0,		// blue
	0xC000C0,		// magenta
	0x00C0C0,		// cyan
	0xC0C0C0,		// white
	0x808080,		// black, high-intensity (dark gray)
	0xFF0000,		// red
	0x00FF00,		// green
	0xFFFF00,		// yellow
	0x0000FF,		// blue
	0xFF00FF,		// magenta
	0x00FFFF,		// cyan
	0xFFFFFF		// white
};

ChAnsiState::ChAnsiState() :
				m_boolAnsiBold( false ),
				m_boolAnsiUnderline( false ),
				m_boolAnsiItalic( false ),
				m_boolAnsiInverse( false ),
				m_boolAnsiStrikethrough( false ),
				m_boolAnsiColor( false ),
				m_luForeColor( constDefColor ),
				m_luBackColor( constDefColor )
{
}


void ChAnsiState::Reset( ChString& strHTML )
{
	//if (m_boolAnsiBold)
	//{
	//	strHTML += "</b>";
	//	m_boolAnsiBold = false;
	//}

	if (m_boolAnsiStrikethrough)
	{
		strHTML += "</strike>";
		m_boolAnsiStrikethrough = false;
	}
	
	if (m_boolAnsiUnderline)
	{
		strHTML += "</u>";
		m_boolAnsiUnderline = false;
	}

	if (m_boolAnsiItalic)
	{
		strHTML += "</i>";
		m_boolAnsiItalic = false;
	}

	if (m_boolAnsiBold || m_boolAnsiColor || m_boolAnsiInverse)
	{
		strHTML += "</font>";
		m_boolAnsiBold = m_boolAnsiColor = m_boolAnsiInverse = false;
	}
											/* ANSI reset sets the terminal to
												white-on-black */
	strHTML += "<body bgcolor=#000000>\n";
	ChooseDefaultColors(strHTML);
}

void ChAnsiState::SetBold( bool boolBold, ChString& strHTML )
{
	if (boolBold && !m_boolAnsiBold)
	{
		//strHTML += "<b>";
		// UE: changed meaning of "bold" to "high-intensity colour"
		if (m_luForeColor <= 7)
			m_luForeColor += 8;
		SetColor( m_luForeColor, m_luBackColor, strHTML );
		m_boolAnsiBold = true;
	}
	else if (!boolBold && m_boolAnsiBold)
	{
		//strHTML += "</b>";
		if (m_luForeColor >= 8 && m_luForeColor <= 15)
			m_luForeColor -= 8;
		SetColor( m_luForeColor, m_luBackColor, strHTML );
		m_boolAnsiBold = false;
	}
}


void ChAnsiState::SetUnderline( bool boolUnderline, ChString& strHTML )
{
	if (boolUnderline && !m_boolAnsiUnderline)
	{
		strHTML += "<u>";
		m_boolAnsiUnderline = true;
	}
	else if (!boolUnderline && m_boolAnsiUnderline)
	{
		strHTML += "</u>";
		m_boolAnsiUnderline = false;
	}
}


void ChAnsiState::SetItalic( bool boolItalic, ChString& strHTML )
{
	if (boolItalic && !m_boolAnsiItalic)
	{
		strHTML += "<i>";
		m_boolAnsiItalic = true;
	}
	else if (!boolItalic && m_boolAnsiItalic)
	{
		strHTML += "</i>";
		m_boolAnsiItalic = false;
	}
}


void ChAnsiState::SetInverse( bool boolInverse, ChString& strHTML )
{
	if (boolInverse && !m_boolAnsiInverse) {
		m_boolAnsiInverse = true;
		SetColor( m_luForeColor, m_luBackColor, strHTML );
	} else if (!boolInverse && m_boolAnsiInverse) {
		m_boolAnsiInverse = false;
		SetColor( m_luForeColor, m_luBackColor, strHTML );
	}
}


void ChAnsiState::SetStrikethrough( bool boolStrikethrough, ChString& strHTML )
{
	if (boolStrikethrough && !m_boolAnsiStrikethrough)
	{
		strHTML += "<strike>";
		m_boolAnsiStrikethrough = true;
	}
	else if (!boolStrikethrough && m_boolAnsiStrikethrough)
	{
		strHTML += "</strike>";
		m_boolAnsiStrikethrough = false;
	}
}


void ChAnsiState::SetForeColor( chuint8 luColor, ChString& strHTML )
{
	m_luForeColor = luColor;
	SetColor( m_luForeColor, m_luBackColor, strHTML );
}


void ChAnsiState::SetBackColor( chuint8 luColor, ChString& strHTML )
{
	m_luBackColor = luColor;
	SetColor( m_luForeColor, m_luBackColor, strHTML );
}


void ChAnsiState::SetColor( chuint8 luForeColor, chuint8 luBackColor,
							ChString& strHTML )
{
	ChString		strTemp;

	if (m_boolAnsiColor)
	{
		strHTML += "</font>";
		m_boolAnsiColor = false;
	}

#ifdef __BORLANDC__
 #pragma warn -csu
#endif
	if ((luForeColor != constDefColor) || (luBackColor != constDefColor))
	{
		ChString		strForeHTML;
		ChString		strBackHTML;

		if (luForeColor != constDefColor)
		{
			strForeHTML.Format( " %cgcolor=\"#%6.6lX\"", m_boolAnsiInverse ? 'b' : 'f',
								colourTable[luForeColor] );
		}

		if (luBackColor != constDefColor)
		{
			strBackHTML.Format( " %cgcolor=\"#%6.6lX\"", m_boolAnsiInverse ? 'f' : 'b',
								colourTable[luBackColor] );
		}
#ifdef __BORLANDC__
 #pragma warn .csu
#endif

		strHTML += "<font" + strForeHTML + strBackHTML + ">";
		m_boolAnsiColor = true;
	}
}

void ChAnsiState::ChooseDefaultColors( ChString& strHTML )
{
	// Assign colours directly instead of through SetForeColor/SetBackColor,
	// as those two produce extra HTML overhead.  I know, that isn't too
	// critical, but it does make for a bit of a performance hit.
	m_luForeColor = 7; m_luBackColor = 0;
	SetColor( m_luForeColor, m_luBackColor, strHTML );
}


/*----------------------------------------------------------------------------
	ChWorldConn class
----------------------------------------------------------------------------*/

ChWorldConn::ChWorldConn( const ChModuleID& idModule,
							ChSocketHandler pHandler,
							ChWorldMainInfo* pMainInfo, chparam userData ) :
				ChConn( pHandler, userData ),
				m_idModule( idModule ),
				m_pMainInfo( pMainInfo ),
				m_state( stateNotConnected ),
				m_textMode( modeText ),
				m_boolRawMode( false ),
				m_boolPuebloEnhanced( false ),
				m_boolTextReceived( false ),
				m_boolAnsiResetReceived( false ),
				m_flTelnetState( telnetStateEcho ),
				m_parseState( parseStateScanning ),
				m_telnetParseState( parseStateScanning ),
				m_iCurrentTermType( 0 )
{
	UpdatePreferences();
											// Cache text out object

	m_pTextOutput = GetMainInfo()->GetTextOutput();

	#if defined( DUMP_TELNET_CODES )
	{
		int		iLoop;

	    for (iLoop = 0; iLoop < 256; iLoop++)
	    {
	    	pstrTelnetLabel[iLoop] = 0;
		}

		pstrTelnetLabel[TN_ECHO]			= "ECHO";
		pstrTelnetLabel[TN_SGA]				= "SGA";
		pstrTelnetLabel[TN_STATUS]			= "STATUS";
		pstrTelnetLabel[TN_TIMING_MARK]		= "TIMING_MARK";
		pstrTelnetLabel[TN_TERMINAL_TYPE]	= "TERMINAL_TYPE";
		pstrTelnetLabel[TN_EOR_OPT]			= "EOR_OPT";
		pstrTelnetLabel[TN_NAWS]			= "NAWS";
		pstrTelnetLabel[TN_TSPEED]			= "TSPEED";
		pstrTelnetLabel[TN_LINEMODE]		= "LINEMODE";
		pstrTelnetLabel[TN_COMPRESS]		= "COMPRESS1";
		pstrTelnetLabel[TN_COMPRESS2]		= "COMPRESS2";
		pstrTelnetLabel[TN_EOR]				= "EOR";
		pstrTelnetLabel[TN_SE]				= "SE";
		pstrTelnetLabel[TN_NOP]				= "NOP";
		pstrTelnetLabel[TN_DATA_MARK]		= "DATA_MARK";
		pstrTelnetLabel[TN_BRK]				= "BRK";
		pstrTelnetLabel[TN_IP]				= "IP";
		pstrTelnetLabel[TN_AO]				= "AO";
		pstrTelnetLabel[TN_AYT]				= "AYT";
		pstrTelnetLabel[TN_EC]				= "EC";
		pstrTelnetLabel[TN_EL]				= "EL";
		pstrTelnetLabel[TN_GA]				= "GA";
		pstrTelnetLabel[TN_SB]				= "SB";
		pstrTelnetLabel[TN_WILL]			= "WILL";
		pstrTelnetLabel[TN_WONT]			= "WONT";
		pstrTelnetLabel[TN_DO]				= "DO";
		pstrTelnetLabel[TN_DONT]			= "DONT";
		pstrTelnetLabel[TN_IAC]				= "IAC";
	}
	#endif	// defined( CH_DEBUG )
}


ChWorldConn::~ChWorldConn()
{
	if (IsConnected())
	{
		TermConnection();
	}
}


void ChWorldConn::AsyncGetHostByName( const ChString& strHost,
										ChSocketAsyncHandler pprocHandler,
										chparam userData )
{
	GetChacoSocket()->AsyncGetHostByName( strHost, pprocHandler,
											userData );
}


void ChWorldConn::Connect( const ChString& strHost, chint16 sPort )
{
	ASSERT( this );
											/* Set flag to indicate that we've
												not yet received any data */
	SetMode( modeText );
											// Start async connect
	SOCKSconnect( strHost, sPort );
}


void ChWorldConn::Connect( chuint32 luAddress, chint16 sPort )
{
										// luAddress is in host order
	ASSERT( this );
										/* Set flag to indicate that we've
											not yet received any data */
	SetMode( modeText );
										// Start async connect

	SOCKSconnect( luAddress, sPort );
}


void ChWorldConn::ProcessOutput()
{											/* Send incoming text to the text
												output module */
	chuint32	luLen;

	ASSERT( this );

	if ((luLen = GetBytesAvailable()) > 0)
	{
		char*	pstrBuffer;
		int		iLen;
		ChString	strOut;

		if (luLen > INT_MAX)
		{
			iLen = INT_MAX;
		}
		else
		{
			iLen = (int)luLen;
		}
											/* Read the contents of the socket
												to the end of 'inputbuf' */

		pstrBuffer = strOut.GetBufferSetLength( iLen + 1 );
		read( pstrBuffer, iLen );
		strOut.ReleaseBuffer( iLen );

		ParseIncomingData(strOut, iLen);

											// Display the text
		OutputBuffer( strOut );

		if (!IsAnyTextReceived())
		{
			SetAnyTextReceived();
			GetMainInfo()->DoAutoLogin();
		}
	}
}


void ChWorldConn::InitConnection()
{
	ChString		strOut;

	m_state = stateConnected;
											/* Make the default text output
												mode HTML, rather than <pre> */
	strOut = "<html>";

											/* Indicate that we will be using
												preformatted text */
	TurnHtmlOff( strOut );
	OutputBuffer( strOut, false );
}


void ChWorldConn::TermConnection()
{
	if (IsConnected())
	{
		ChString		strOut;

		m_state = stateNotConnected;
		TurnHtmlOn( strOut );
											/* Indicating that we are finished
												using preformatted text */
		OutputBuffer( strOut );
	}
}


void ChWorldConn::SendWorldCommand( const ChString& strCommand,
									bool boolUserCommand )
{
	SendLine( strCommand );

	if (boolUserCommand)
	{
		EchoState	echo = GetMainInfo()->GetEchoState();

		if (m_boolEcho && (echoOn == echo))
		{
			ChString		strCommandOut( strCommand );
			ChString		strOut;

			TurnHtmlOn( strOut );

			if (m_boolBold)
			{
				strOut += "<b>";
			}

			if (m_boolItalic)
			{
				strOut += "<i>";
			}
			
											/* Append the users' text to the
												output buffer */

			Display( strOut, strCommandOut, true );

			if (m_boolItalic)
			{
				strOut += "</i>";
			}

			if (m_boolBold)
			{
				strOut += "</b>";
			}

			strOut += "<br>";

			TurnHtmlOff( strOut );
			OutputBuffer( strOut, false );
		}
		else if (m_boolEcho && (echoOn != echo))
		{									// Just send a line break
			OutputBuffer( "\n" );
		}
	}
}


void ChWorldConn::Display( const ChString& strText )
{
	ChString		strOut;

	TurnHtmlOn( strOut );
	strOut += "<b>";

	Display( strOut, strText );
	
	strOut += "</b><br>";
	TurnHtmlOff( strOut );

	OutputBuffer( strOut, false );
}


void ChWorldConn::Display( ChString& strOut, const ChString& strText, bool boolUserCommand /*=false*/ )
{
	ChString		strTextOut( strText );
	chuint32	luBackColor = m_ansiState.GetBackColor();
	chuint32	luForeColor = GetTextOutput()->GetDefForeColor();
	ChString		strColor;

	if (luForeColor == luBackColor)
	{
		luForeColor = (~luForeColor) & 0xffffff;
	}
	strColor.Format( "#%06lx", luForeColor );

	strOut += "<font text=\"" + strColor + "\">";

								/* Strip out HTML from the users'
									text and append it to the
									output buffer */

	ChHtmlWnd::EscapeForHTML( strTextOut );
	if (boolUserCommand) {
		// UE: Displaying user text; we want to split up the display properly
		//     into lines, so it doesn't all run together.
		TranslateNewlinesToBreaks(strTextOut);
		//strTextOut.Replace("\r\n", "<br>");
	}
	strOut += strTextOut;

	strOut += "</font>";
}


void ChWorldConn::UpdatePreferences()
{
	ChRegistry		worldPrefsReg( WORLD_PREFS_GROUP );

	worldPrefsReg.ReadBool( WORLD_PREFS_ECHO, m_boolEcho, true );
	worldPrefsReg.ReadBool( WORLD_PREFS_ECHO_BOLD, m_boolBold, true );
	worldPrefsReg.ReadBool( WORLD_PREFS_ECHO_ITALIC, m_boolItalic, false );
	worldPrefsReg.ReadBool( WORLD_PREFS_ALLOWMCCP, m_boolAllowMCCP, WORLD_PREFS_ALLOWMCCP_DEF );
}


/*----------------------------------------------------------------------------
	ChWorldConn protected methods
----------------------------------------------------------------------------*/

void ChWorldConn::ParseIncomingData( ChString& strData, int iLen )
{
	// This routine parses the incoming (possibly compressed) data, and also
	// handles Telnet command sequences.
	m_mccp.receive(strData, iLen);

	if (m_mccp.error())
	{
		ChString strMessage;
		LOADSTRING( IDS_DECOMPRESSION_ERROR, strMessage );
		GetMainInfo()->GetCore()->GetFrameWnd()->MessageBox( strMessage );
		TelnetSend(TN_DONT, TN_COMPRESS);
		TelnetSend(TN_DONT, TN_COMPRESS2);
		// note that most servers don't actually pay attention to that, but it's
		// about the best we can do...
		GetMainInfo()->GetCore()->SetStatusPaneText(ChCore::paneCompression, _T(""));
		m_mccp.negotiated(ChMCCP::modeUncompressed);
		return;
	}

	UpdateCompressionPanel();

	int available = m_mccp.pending();
	if (!available)
	{
		// no incoming data (decompressor might not have enough yet)
		strData = _T("");
		return;
	}

	iLen = m_mccp.get( strData.GetBuffer(available + 2), available + 1 );
	strData.ReleaseBuffer( iLen );
	ASSERT( iLen == available );

	if (0 < (available = m_mccp.pending()))
	{
		// retrieving some decompressed text freed up some more, so read that in too
		ChString additional;
		int extraLen = m_mccp.get( additional.GetBuffer(available + 2), available + 1 );
		additional.ReleaseBuffer( extraLen );

		//strData.Append(additional, extraLen);
		strData += additional;
		iLen += extraLen;
	}

	ChBufferString strParsedBuffer;
	const unsigned char*	pstrOriginText;
	bool boolTerminateParsing = false;
	pstrOriginText = (const unsigned char*)(LPCSTR)strData;

	while ((iLen > 0) && !boolTerminateParsing)
	{
		switch (m_telnetParseState)
		{
			case parseStateScanning:
			{
				if (*pstrOriginText == TN_IAC)		// 0xFF
				{
					m_telnetParseState = parseTelnetIAC;
					break;
				}
				else
				{
					strParsedBuffer += *pstrOriginText;
				}
				break;
			}

			case parseTelnetIAC:
			{
				m_iTelnetCmd = *pstrOriginText;

				if (TN_IAC == m_iTelnetCmd)
				{
					// UE: IAC IAC is just an escape meaning we want a regular 255 data byte
					TelnetDisplay("recv IAC IAC\n");
					strParsedBuffer += *pstrOriginText;
					m_telnetParseState = parseStateScanning;
				}
				else if (TN_NOP == m_iTelnetCmd)
				{
	  			m_parseState = parseStateScanning;
				}
				else if ((TN_WILL == m_iTelnetCmd) || (TN_WONT == m_iTelnetCmd) ||
					(TN_DO == m_iTelnetCmd) || (TN_DONT == m_iTelnetCmd))
				{
					m_telnetParseState = parseTelnetCmd;
				}
				else if ((TN_GA == m_iTelnetCmd) || (TN_EOR == m_iTelnetCmd))
				{
											// This is definitely a prompt

					#if defined( DUMP_TELNET_CODES )
					{
						ChString		strSequence;
						ChString		strOut;

						strSequence.Format( "rcvd IAC %s\n",
											pstrTelnetLabel[m_iTelnetCmd] );
						TelnetDisplay( strSequence );
					}
					#endif	// defined( DUMP_TELNET_CODES )

											// State goes back to 'scanning'

					m_telnetParseState = parseStateScanning;
				}
				else if (TN_SB == m_iTelnetCmd)
				{
					// Start of telnet subnegotiation
					m_telnetParseState = parseTelnetSubnegotiateCmd;
					m_strSubnegotiation.Empty();
				}
				else
				{
					strParsedBuffer += (char)TN_IAC;
					strParsedBuffer += *pstrOriginText;

					m_telnetParseState = parseStateScanning;
				}
				break;
			}

			case parseTelnetCmd:
			{
				DoTelnetCmd( m_iTelnetCmd, (unsigned char)*pstrOriginText );
				m_telnetParseState = parseStateScanning;
				break;
			}

			case parseTelnetSubnegotiateCmd:	//UE
			{
				// The first byte of a subnegotiation is the command it applies to
				m_iTelnetCmd = *pstrOriginText;
				TelnetReceive(TN_SB, m_iTelnetCmd);
				m_telnetParseState = parseTelnetSubnegotiate;
				break;
			}

			case parseTelnetSubnegotiate:		//UE
			{
				if (TN_IAC == *pstrOriginText)
				{
					// an IAC in the subnegotiation is handled specially
					m_telnetParseState = parseTelnetSubnegotiateIAC;
				}
				else
				{
					// any other byte is simply remembered as part of the negotiation
					m_strSubnegotiation += *pstrOriginText;
				}
				break;
			}

			case parseTelnetSubnegotiateIAC:	//UE
			{
				if (TN_SE == *pstrOriginText)
				{
					// IAC SE, means we've reached the end of the negotiation
					DoTelnetSubnegotiation(m_iTelnetCmd, m_strSubnegotiation);
					m_telnetParseState = parseStateScanning;
				}
				else if (TN_IAC == *pstrOriginText)
				{
					// IAC IAC, means we want a data byte 255
					m_strSubnegotiation += '\xFF';
					m_telnetParseState = parseTelnetSubnegotiate;
				}
				else
				{
					// technically, this case is illegal.  However, in the
					// interests of compatibility we'll just assume that the
					// sender forgot to properly escape the IAC byte, and it's
					// actually just another data byte.
					m_strSubnegotiation += '\xFF';
					m_strSubnegotiation += *pstrOriginText;
					m_telnetParseState = parseTelnetSubnegotiate;
				}
				break;
			}

			default:
			{
				TRACE( "Error in parse state!\n" );
				ASSERT( false );
				break;
			}
		}

		// move on to next character
		if (iLen > 0)
		{
			pstrOriginText++;
			iLen--;
		}
	}

	strData = CString(strParsedBuffer);
	ParseText( strData, (GetMode() == modeHtml) );
}

void ChWorldConn::UpdateCompressionPanel()
{
	if (m_mccp.compressing())
	{
		if (!(m_flTelnetState & telnetStateMCCPOn))
		{
			// input is now compressed
			m_flTelnetState |= telnetStateMCCPOn;
			if (m_flTelnetState & telnetStateMCCP1)
			{
				GetMainInfo()->GetCore()->SetStatusPaneText( ChCore::paneCompression, _T("MCCP1") );
			}
			else if(m_flTelnetState & telnetStateMCCP2)
			{
				GetMainInfo()->GetCore()->SetStatusPaneText( ChCore::paneCompression, _T("MCCP2") );
			}
			else
			{
				GetMainInfo()->GetCore()->SetStatusPaneText( ChCore::paneCompression, _T("????") );
			}
		}
	}
	else
	{
		if (m_flTelnetState & telnetStateMCCPOn)
		{
			// input no longer compressed
			m_flTelnetState &= ~telnetStateMCCPOn;
			GetMainInfo()->GetCore()->SetStatusPaneText( ChCore::paneCompression, _T("") );
		}
	}
}

void ChWorldConn::ParseText( ChString& strText, bool boolNewlinesToBreaks )
{
	ChString			strWorking;
	const unsigned char*	pstrOriginText;
	int				iStartOfLine = 0;
	ChBufferString	strParsedBuffer;
	bool boolTerminateParsing = false;

	pstrOriginText = (const unsigned char*)(LPCSTR)strText;

	while (*pstrOriginText && !boolTerminateParsing)
	{
		bool	boolMoveToNextChar = true;

		switch (m_parseState)
		{
			case parseStateScanning:
			{
				switch( (unsigned char)*pstrOriginText )
				{
					// UE: new control character begin
					case BELL:
					{
						// emit alert tag
						ChString		strTemp;
						TurnHtmlOn( strTemp );
						strTemp += "<img xch_alert>";
						TurnHtmlOff( strTemp );
						
						strParsedBuffer += strTemp;
						break;
					}
					// UE: new control character end

					case ESCAPE:
					{
						m_parseState = parseStateEscape;
						break;
					}

					case '\n':
					{
						if (*(pstrOriginText + 1) == '\r')
						{
							pstrOriginText++;
						}

						ParseEndOfLine( strParsedBuffer, iStartOfLine, true );

											// Append a newline or <br>

						if (boolNewlinesToBreaks)
						{
							strParsedBuffer += "<br>";
						}
						else
						{
							strParsedBuffer += '\n';
						}

						iStartOfLine = strParsedBuffer.GetLength();
						break;
					}

					case '\r':
					{
						if (*(pstrOriginText + 1) == '\n')
						{
							pstrOriginText++;
						}

						ParseEndOfLine( strParsedBuffer, iStartOfLine, true );

											// Append a newline or <br>

						if (boolNewlinesToBreaks)
						{
							strParsedBuffer += "<br>";
						}
						else
						{
							strParsedBuffer += '\n';
						}

						iStartOfLine = strParsedBuffer.GetLength();
						break;
					}

					default:
					{
						if (isprint( *pstrOriginText ) || ('\t' == *pstrOriginText))
						{
							strParsedBuffer += *pstrOriginText;
						}
						break;
					}
				}
				break;
			}

			case parseStateEscape:
			{
				switch( *pstrOriginText )
				{
					case '[':
					{
						m_parseState = parseStateAnsi;
						break;
					}

					default:
					{
						strParsedBuffer += ESCAPE;
						strParsedBuffer += *pstrOriginText;

											/* Set the state back to being
												between sequences */

						m_parseState = parseStateScanning;
						break;
					}
				}
				break;
			}

			case parseStateAnsi:
			{
				bool		boolEndOfPacket;

				ASSERT( *pstrOriginText );
											/* Search for the end of the ANSI
												sequence (which may contain
												digits or semi-colons) */

				while ((ANSI_SEPARATOR == *pstrOriginText) ||
						isdigit( *pstrOriginText ))
				{
					m_strAnsiSequence += *pstrOriginText;
					pstrOriginText++;
				}
											/* Figure out if we've hit the end
												of the packet before
												encountering the end of the
												ANSI sequence */

				boolEndOfPacket = (0 == *pstrOriginText);
				if (!boolEndOfPacket)
				{							/* Move to processing the ANSI
												terminating character */

					m_parseState = parseStateAnsiTerm;
				}

				boolMoveToNextChar = false;
				break;
			}

			case parseStateAnsiTerm:
			{
				char	cTerm = *pstrOriginText;
				bool	boolValidSequence = true;

				if (cTerm == '\n' || cTerm == '\r' || cTerm == ESCAPE)
				{
											/* Don't strip off newline or
												ESCAPE terms */
					boolMoveToNextChar = false;
					boolValidSequence = false;
				}
				else if (m_strAnsiSequence.IsEmpty())
				{
					#if defined( DUMP_ANSI_CODES )
					{
						ChString		strSequence;

						strSequence.Format( "ANSI(unknown: [%c)",
											(char)*pstrOriginText );
						AnsiDisplay( strSequence );
					}
					#endif	// defined( DUMP_ANSI_CODES )

					boolValidSequence = false;
				}

				if (boolValidSequence)
				{
					ProcessAnsiCodes( cTerm, m_strAnsiSequence,
										strParsedBuffer );
				}
											// State goes back to 'scanning'
				m_strAnsiSequence = "";
				m_parseState = parseStateScanning;
				break;
			}

			default:
			{
				TRACE( "Error in parse state!\n" );
				ASSERT( false );
				break;
			}
		}

		if (boolMoveToNextChar && *pstrOriginText)
		{
			pstrOriginText++;
		}
	}

	ParseEndOfLine( strParsedBuffer, iStartOfLine, false );

											// Return the parsed buffer
	strText = CString(strParsedBuffer);
}


void ChWorldConn::ParseEndOfLine( ChBufferString& strBuffer, int iStartOfLine,
									bool boolEndOfLine )
{
	register int		iBufferLen = strBuffer.GetLength();

	ASSERT( iBufferLen >= iStartOfLine );

	if (iStartOfLine != iBufferLen)
	{
		const char*		pstrBuffer = strBuffer;
		int				iLen = iBufferLen - iStartOfLine;
		
		pstrBuffer += iStartOfLine;
		AddToTextBlock( pstrBuffer, iLen, boolEndOfLine );
	}
}


void ChWorldConn::ProcessAnsiCodes( char cCodeType, const char* pstrCodes,
									ChBufferString& strBuffer )
{
	ChString		strAnsiHTML;

	cCodeType = (char)tolower( cCodeType );
	if ((cCodeType != ANSI_MODE) && (cCodeType != ANSI_ERASE))
	{
											/* Right now we only support two
												types of ANSI sequence */
		return;
	}

	if (*pstrCodes == 0)
	{
		// UE: support single default parameter
		ProcessAnsiCode( cCodeType, -1, strAnsiHTML );
	}

	while (*pstrCodes != 0)
	{
		if (*pstrCodes == ANSI_SEPARATOR)
		{
			// UE: a separator on its own means that we want a default parameter!
			ProcessAnsiCode( cCodeType, -1, strAnsiHTML );
			++pstrCodes;
		}
		else if (isdigit( *pstrCodes ))
		{									// There's a code here
			int		iCode = 0;
			ChString	strAnsiOutput;

			while (isdigit( *pstrCodes ))
			{
				iCode = (iCode * 10) + (int)(*pstrCodes - '0');
				pstrCodes++;
			}

			ProcessAnsiCode( cCodeType, iCode, strAnsiHTML );

			// UE: strip single following separator, if any
			if (*pstrCodes == ANSI_SEPARATOR)
			{
				++pstrCodes;
			}
		}
	}

	if (strAnsiHTML.GetLength() > 0)
	{
		ChString		strTemp;
											// Add HTML
		TurnHtmlOn( strTemp );
		strTemp += strAnsiHTML;
		TurnHtmlOff( strTemp );

		strBuffer += strTemp;
	}
}


void ChWorldConn::ProcessAnsiCode( char cCodeType, int iCode,
									ChString& strOutput )
{
	ChString		strCodeHTML;

	switch( cCodeType )
	{
		case ANSI_MODE:
		{
			bool bBold = m_ansiState.GetBold();
			switch( iCode )
			{
				case 0:						// Reset all
				case -1:					// (this is the default mode)
				{
					AnsiDisplay( "ANSI(reset)" );

					if (!m_boolAnsiResetReceived)
					{
											/* The first time we get an ANSI
												reset, switch all black objects
												in the file to white */

						GetTextOutput()->RemapColor( ChAnsiState::colourTable[0],
																				 ChAnsiState::colourTable[7] );
						m_boolAnsiResetReceived = true;
					}
					m_ansiState.Reset( strCodeHTML );
					break;
				}

				case 1:						// High intensity
				{
					AnsiDisplay( "ANSI(bold)" );
					m_ansiState.SetBold( true, strCodeHTML );
					break;
				}

				case 2:						// Low intensity (original Pueblo)
				case 22:					// ANSI standard low intensity (UE)
				{
					AnsiDisplay( "ANSI(nonbold)" );
					m_ansiState.SetBold( false, strCodeHTML );
					break;
				}

				case 3:						// Italic
				{
					AnsiDisplay( "ANSI(italic)" );
					m_ansiState.SetItalic( true, strCodeHTML );
					break;
				}

				case 4:						// Underline
				{
					AnsiDisplay( "ANSI(underline)" );
					m_ansiState.SetUnderline( true, strCodeHTML );
					break;
				}
				
				// UE: new codes begin
				case 7:						// Inverse
				{
					AnsiDisplay( "ANSI(inverse)" );
					m_ansiState.SetInverse( true, strCodeHTML );
					break;
				}
				
				case 9:						// Strikethrough
				{
					AnsiDisplay( "ANSI(strikethrough)" );
					m_ansiState.SetStrikethrough( true, strCodeHTML );
					break;
				}
				
				case 23:					// Italic off
				{
					AnsiDisplay( "ANSI(nonitalic)" );
					m_ansiState.SetItalic( false, strCodeHTML );
					break;
				}
				
				case 24:					// Underline off
				{
					AnsiDisplay( "ANSI(nonunderline)" );
					m_ansiState.SetUnderline( false, strCodeHTML );
					break;
				}
				
				case 27:					// Inverse off
				{
					AnsiDisplay( "ANSI(noninverse)" );
					m_ansiState.SetInverse( false, strCodeHTML );
					break;
				}
				
				case 29:					// Strikethrough off
				{
					AnsiDisplay( "ANSI(nonstrikethrough)" );
					m_ansiState.SetStrikethrough( false, strCodeHTML );
					break;
				}
				// UE: new codes end

				case 30:					// Black foreground
				{
					AnsiDisplay( "ANSI(fg-black)" );
					m_ansiState.SetForeColor( bBold ? 8 : 0, strCodeHTML );
					break;
				}

				case 31:					// Red foreground
				{
					AnsiDisplay( "ANSI(fg-red)" );
					m_ansiState.SetForeColor( bBold ? 9 : 1, strCodeHTML );
					break;
				}

				case 32:					// Green foreground
				{
					AnsiDisplay( "ANSI(fg-green)" );
					m_ansiState.SetForeColor( bBold ? 10 : 2, strCodeHTML );
					break;
				}

				case 33:					// Yellow foreground
				{
					AnsiDisplay( "ANSI(fg-yellow)" );
					m_ansiState.SetForeColor( bBold ? 11 : 3, strCodeHTML );
					break;
				}

				case 34:					// Blue foreground
				{
					AnsiDisplay( "ANSI(fg-blue)" );
					m_ansiState.SetForeColor( bBold ? 12 : 4, strCodeHTML );
					break;
				}

				case 35:					// Magenta foreground
				{
					AnsiDisplay( "ANSI(fg-magenta)" );
					m_ansiState.SetForeColor( bBold ? 13 : 5, strCodeHTML );
					break;
				}

				case 36:					// Cyan foreground
				{
					AnsiDisplay( "ANSI(fg-cyan)" );
					m_ansiState.SetForeColor( bBold ? 14 : 6, strCodeHTML );
					break;
				}

				case 37:					// White foreground
				{
					AnsiDisplay( "ANSI(fg-white)" );
					m_ansiState.SetForeColor( bBold ? 15 : 7, strCodeHTML );
					break;
				}

				case 39:					// Default foreground (UE 2.53)
				{
					AnsiDisplay( "ANSI(fg-default)" );
					m_ansiState.SetForeColor( bBold ? 15 : 7, strCodeHTML );
					break;
				}

				case 40:					// Black background
				{
					AnsiDisplay( "ANSI(bg-black)" );
					m_ansiState.SetBackColor( 0, strCodeHTML );
					break;
				}

				case 41:					// Red background
				{
					AnsiDisplay( "ANSI(bg-red)" );
					m_ansiState.SetBackColor( 9, strCodeHTML );
					break;
				}

				case 42:					// Green background
				{
					AnsiDisplay( "ANSI(bg-green)" );
					m_ansiState.SetBackColor( 10, strCodeHTML );
					break;
				}

				case 43:					// Yellow background
				{
					AnsiDisplay( "ANSI(bg-yellow)" );
					m_ansiState.SetBackColor( 11, strCodeHTML );
					break;
				}

				case 44:					// Blue background
				{
					AnsiDisplay( "ANSI(bg-blue)" );
					m_ansiState.SetBackColor( 12, strCodeHTML );
					break;
				}

				case 45:					// Magenta background
				{
					AnsiDisplay( "ANSI(bg-magenta)" );
					m_ansiState.SetBackColor( 13, strCodeHTML );
					break;
				}

				case 46:					// Cyan background
				{
					AnsiDisplay( "ANSI(bg-cyan)" );
					m_ansiState.SetBackColor( 14, strCodeHTML );
					break;
				}

				case 47:					// White background
				{
					AnsiDisplay( "ANSI(bg-white)" );
					m_ansiState.SetBackColor( 15, strCodeHTML );
					break;
				}

				case 49:					// Default background (UE 2.53)
				{
					AnsiDisplay( "ANSI(bg-default)" );
					m_ansiState.SetBackColor( 0, strCodeHTML );
					break;
				}

				default:
				{
					break;
				}
			}
			break;
		}

		case ANSI_ERASE:
		{
			switch( iCode )
			{
				case 2:						// Clear display
				{
					AnsiDisplay( "ANSI(clear)" );
					// UE: actually implemented
					strCodeHTML += "<xch_page clear=text>";
					break;
				}

				case -1:					// the default is 0, which is unsupported
				default:
				{
					break;
				}
			}
			break;
		}
	}

	if (strCodeHTML.GetLength() > 0)
	{
		strOutput += strCodeHTML;
	}
}

#if defined( DUMP_TELNET_CODES )
ChString ChWorldConn::GetTelnetLabel( int iCommand )
{
	ChString strLabel;

	if (pstrTelnetLabel[iCommand])
	{
		strLabel = pstrTelnetLabel[iCommand];
	}
	else 
	{
		strLabel.Format( "%d", iCommand );
	}
	return strLabel;
}
#endif

void ChWorldConn::TelnetSend( int iCommand, int iOption )
{
	char	cSequence[3];

	cSequence[0] = (char)TN_IAC;
	cSequence[1] = (char)iCommand;
	cSequence[2] = (char)iOption;

	SendBlock( cSequence, 3 );

	#if defined( DUMP_TELNET_CODES )
	{
		ChString		strSequence;
		strSequence.Format( "sent IAC %s %s\n", (LPCSTR)GetTelnetLabel(iCommand), (LPCSTR)GetTelnetLabel(iOption) );
		TelnetDisplay( strSequence );
	}
	#endif	// defined( DUMP_TELNET_CODES )
}

void ChWorldConn::TelnetReceive( int iCommand, int iOption )
{
	#if defined( DUMP_TELNET_CODES )
	{
		ChString		strSequence;
		strSequence.Format( "recv IAC %s %s\n", (LPCSTR)GetTelnetLabel(iCommand), (LPCSTR)GetTelnetLabel(iOption) );
		TelnetDisplay( strSequence );
	}
	#endif	// defined( DUMP_TELNET_CODES )
}

void ChWorldConn::DoTelnetCmd( int iCommand, int iOption )
{
	/*
		From RFC 854:

		Telnet protocols always start with the IAC (255) character.  Following
		this character may be a number of bytes, but we only pay attention to
		the following:

			WILL		251
			WON'T		252
			DO			253
			DON'T		254

		These are commands, sent from one end to the other.  They say that the
		sender either WILL or WON'T do something, or that the receiver should
		either DO or DON'T do something.  The third byte indicates what should
		be done.  We currently pay attention to the following:

			ECHO			1
			TERMINAL_TYPE	24
			COMPRESS				85	(UE 2.54)
			COMPRESS2				86	(UE 2.54)

		Others are ignored.
	*/

	switch( iCommand )
	{
		case TN_WILL:
		{
			TelnetReceive( TN_WILL, iOption );

			switch( iOption )
			{
				case TN_ECHO:
				{
          if (m_flTelnetState & telnetStateEcho)
          {
						// Stop local echo, and acknowledge
						GetMainInfo()->SetEchoState( echoTelnetOff );
            m_flTelnetState &= ~telnetStateEcho;

            TelnetSend( TN_DO, TN_ECHO );
          }
          else
          {
						// We already said DO ECHO, so ignore WILL ECHO
          }
					break;
				}

        case TN_EOR_OPT:
				{
					if (!(m_flTelnetState & telnetStateEOR))
					{
						m_flTelnetState |= telnetStateEOR;
						TelnetSend( TN_DO, TN_EOR_OPT );
					}
					else
					{
						// We already said DO EOR_OPT, so ignore WILL EOR_OPT
					}
					break;
				}

				case TN_COMPRESS:	//UE
				{
					if (m_boolAllowMCCP)
					{
						if (!(m_flTelnetState & (telnetStateMCCP1 | telnetStateMCCP2)))
						{
							m_flTelnetState |= telnetStateMCCP1;
							TelnetSend( TN_DO, iOption );
							m_mccp.negotiated(ChMCCP::modeCompressV1);
						}
						else
						{
							// We've already accepted either COMPRESS or COMPRESS2, so ignore WILL COMPRESS
						}
					}
					else
					{
						// User doesn't want to allow use of MCCP
						TelnetSend( TN_DONT, iOption );
					}
					break;
				}

				case TN_COMPRESS2:	//UE
				{
					if (m_boolAllowMCCP)
					{
						if (!(m_flTelnetState & (telnetStateMCCP1 | telnetStateMCCP2)))
						{
							m_flTelnetState |= telnetStateMCCP2;
							TelnetSend( TN_DO, iOption );
							m_mccp.negotiated(ChMCCP::modeCompressV2);
						}
						else
						{
							// We've already accepted either COMPRESS or COMPRESS2, so ignore WILL COMPRESS2
						}
					}
					else
					{
						// User doesn't want to allow use of MCCP
						TelnetSend( TN_DONT, iOption );
					}
					break;
				}

				default:
				{							// Don't accept other WILL offers

					TelnetSend( TN_DONT, iOption );
					break;
				}
			}
			break;
		}

		case TN_WONT:
		{
			TelnetReceive( TN_WONT, iOption );

			switch( iOption )
			{
				case TN_ECHO:
				{
          if (m_flTelnetState & telnetStateEcho)
          {
						// We're already echoing, so ignore WONT ECHO
          }
          else
          {						// Resume local echo, and acknowledge
						GetMainInfo()->SetEchoState( echoOn );
            m_flTelnetState |= telnetStateEcho;

            TelnetSend( TN_DONT, TN_ECHO );
	        }
					break;
				}
	
	      case TN_EOR_OPT:
				{
					if (!(m_flTelnetState & telnetStateEOR))
					{
                        // We're in DONT EOR_OPT state, ignore WONT EOR_OPT
					}
					else
					{						// Acknowledge

						m_flTelnetState &= ~telnetStateEOR;
						TelnetSend( TN_DONT, TN_EOR_OPT );
					}
				}

				case TN_COMPRESS:	//UE
				{
					if (!(m_flTelnetState & telnetStateMCCP1))
					{
								// We're in DONT COMPRESS state, ignore WONT COMPRESS
					}
					else
					{			// Acknowledge
						m_flTelnetState &= ~telnetStateMCCP1;
						TelnetSend( TN_DONT, TN_COMPRESS );
						m_mccp.negotiated(ChMCCP::modeUncompressed);
					}
					break;
				}

				case TN_COMPRESS2:	//UE
				{
					if (!(m_flTelnetState & telnetStateMCCP2))
					{
								// We're in DONT COMPRESS2 state, ignore WONT COMPRESS2
					}
					else
					{			// Acknowledge
						m_flTelnetState &= ~telnetStateMCCP2;
						TelnetSend( TN_DONT, TN_COMPRESS2 );
						m_mccp.negotiated(ChMCCP::modeUncompressed);
					}
					break;
				}

				default:
				{
					break;
				}
			}
			break;
		}

		case TN_DO:
		{
			TelnetReceive( TN_DO, iOption );

			switch( iOption )
			{
				case TN_TERMINAL_TYPE:		//UE
				{
					if (!(m_flTelnetState & telnetStateTType))
					{
						m_flTelnetState |= telnetStateTType;
						m_iCurrentTermType = 0;
						TelnetSend( TN_WILL, iOption );
					}
					else
					{
						// We've already acknowledged TTYPE, so ignore DO TTYPE
					}
					break;
				}

				default:
				{
					// Refuse all other DO requests
					TelnetSend( TN_WONT, iOption );
					break;
				}
			}
			break;
		}

		case TN_DONT:
		{
			TelnetReceive( TN_DONT, iOption );

			switch( iOption )
			{
				case TN_TERMINAL_TYPE:	//UE
				{
					if (!(m_flTelnetState & telnetStateTType))
					{
								// We're in WONT TTYPE state, ignore DONT TTYPE
					}
					else
					{			// Acknowledge
						m_flTelnetState &= ~telnetStateTType;
						TelnetSend( TN_WONT, TN_TERMINAL_TYPE );
					}
					break;
				}
		
				default:
				{
					// Ignore all other DONT requests (we're
					// already in the DONT state)
					break;
				}
			}
			break;
		}

		default:
		{
			break;
		}
	}
}

void ChWorldConn::DoTelnetSubnegotiation( int iCommand, const ChString& data )
{
	switch( iCommand )
	{
		case TN_TERMINAL_TYPE:
		{
			// the only data we accept is a send request.
			if (m_flTelnetState & telnetStateTType)
			{
				if (data.GetLength() == 1)
				{
					if (TN_SEND == data[0])
					{
						DoSendTerminalType();
						return;
					}
				}
			}
			break;
		}
	}

	// we don't know what that is (or we don't care), so just ignore it.
	#if defined( DUMP_TELNET_CODES )
	ChString strMessage;
	strMessage.Format("neg %s: ", (LPCSTR)GetTelnetLabel(iCommand));
	for(int byteIndex = 0; byteIndex < data.GetLength(); ++byteIndex)
	{
		strMessage.AppendFormat("%02X ", data[byteIndex]);
	}
	strMessage.AppendChar('\n');
	TelnetDisplay( strMessage );
	#endif
}

void ChWorldConn::DoSendTerminalType()
{
	// The world can cause this method to be called as much as it likes; each time,
	// it returns the next terminal type in our compatibility list.  Once we reach
	// the end, the next call will repeat the same type again.  A further call will
	// return to the start of the list and send the first type.
	TelnetDisplay( "recv SEND TERMINAL_TYPE req\n" );

	const char *termType = pstrTerminalTypes[m_iCurrentTermType++];
	if (termType == NULL)
	{
		termType = pstrTerminalTypes[m_iCurrentTermType - 2];	// the one before the NULL
		m_iCurrentTermType = 0;
	}

	ChString reply;
	reply.Format("%c%c%c%c%s%c%c", TN_IAC, TN_SB, TN_TERMINAL_TYPE, TN_IS, termType, TN_IAC, TN_SE);
	SendBlock((LPCSTR)reply, reply.GetLength());

	#if defined( DUMP_TELNET_CODES )
	ChString strMessage;
	strMessage.Format("sent %s IS %s\n", (LPCSTR)GetTelnetLabel(TN_TERMINAL_TYPE), termType);
	TelnetDisplay( strMessage );
	#endif
}

void ChWorldConn::TranslateNewlinesToBreaks( ChString& strOut )
{
	int		iIndex;

	iIndex = strOut.Find( '\n' );
	if (-1 != iIndex)
	{
		ChString		strReturn;

		do
		{
			int		iCopyChars = iIndex;

			if ((iIndex > 0) && ('\r' == strOut[iIndex - 1]))
			{
				iCopyChars--;
			}

			strReturn += strOut.Left( iCopyChars ) + "<br>";
			strOut = strOut.Mid( iIndex + 1 );

			iIndex = strOut.Find( '\n' );

		} while (-1 != iIndex);
											// Append whatever's left
		strReturn += strOut;
											// Now return the processed string
		strOut = strReturn;
	}
}


void ChWorldConn::AddToTextBlock( const char* pstrData, int iDataLen,
									bool boolEndOfLine )
{
	int				iOriginalLen = m_strCurrentLine.GetLength();
	int				iLen = iOriginalLen;
	char*			pstrCurrentLine;

	iLen += iDataLen + 1;
	pstrCurrentLine = m_strCurrentLine.GetBuffer( iLen );
	pstrCurrentLine += iOriginalLen;

	strncpy( pstrCurrentLine, pstrData, iDataLen );
	pstrCurrentLine += iDataLen;
	*pstrCurrentLine = 0;
	m_strCurrentLine.ReleaseBuffer();
											/* Process buffer on end-of-line or
												on grossly large lines */

	if (boolEndOfLine || (m_strCurrentLine.GetLength() > MAX_LINE_LEN))
	{
		OnTextLine( m_strCurrentLine );
		m_strCurrentLine = "";
	}
}


void ChWorldConn::OnTextLine( const ChString& strLine )
{
	ChWorldMainInfo*	pInfo = GetMainInfo();
	ChString				strWorking( strLine );

	if (!IsPuebloEnhanced())
	{										/* If we're not Pueblo Enhanced, we
												keep hoping the world will tell
												us that they are */
		ChVersion	ver;

		if (pInfo->LookForPuebloEnhanced( strWorking, ver ))
		{
											// Yay!  They are!  Happy happy!
			SetPuebloEnhanced( true, ver );
		}
	}

	if (pInfo->WantTextLines())
	{
		if (IsPuebloEnhanced())
		{
			ChHtmlWnd::RemoveEntities( strWorking, strWorking );
		}
											// Process TinTin actions
		pInfo->OnTextLine( strWorking );
	}
}


void ChWorldConn::SetPuebloEnhanced( bool boolEnhanced,
										const ChVersion& versEnhanced )
{
	m_boolPuebloEnhanced = boolEnhanced;

	if (m_boolPuebloEnhanced)
	{
		SetPuebloEnhancedVersion( versEnhanced );
	}
}


// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
// Revision 1.1.1.1  2003/02/03 18:53:19  uecasm
// Import of source tree as at version 2.53 release.
//
