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
					Support for MCCP added.

------------------------------------------------------------------------------

	Contains the interface for the ChWorldConn class.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHWCONN_H )
#define _CHWCONN_H

#include <ChConn.h>

#include "ChTextOutput.h"
#include "ChMCCP.h"

class ChWorldMainInfo;

/*----------------------------------------------------------------------------
	Switches
----------------------------------------------------------------------------*/

#if defined( CH_DEBUG )
	#define DUMP_TELNET_CODES	1
	#define DUMP_ANSI_CODES		1
#endif	// defined( CH_DEBUG )


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

//#define COLOR_BRIGHT_WHITE		0xffffff
//#define COLOR_WHITE				0xc0c0c0
//#define COLOR_BLACK				0x000000


/*----------------------------------------------------------------------------
	Types
----------------------------------------------------------------------------*/

typedef enum { stateNotConnected, stateWaitingForAddress,
				stateConnected } ConnectState;

typedef enum { parseStateScanning, parseStateEscape, parseStateAnsi,
				parseStateAnsiTerm, parseTelnetIAC, parseTelnetCmd,
				parseTelnetSubnegotiate, parseTelnetSubnegotiateCmd, parseTelnetSubnegotiateIAC
					} ParseState;


/*----------------------------------------------------------------------------
	ChBufferString class
----------------------------------------------------------------------------*/

class ChBufferString
{
	public:
		enum { initBufferStringSize = 1024, growBufferStringSize = 512 };

	public:
		ChBufferString();
		ChBufferString( const ChString& strData );

		~ChBufferString();

		inline operator ChString() const
				{
					ChString		strTemp( (const char*)m_string,
											m_iActualSize );

					return strTemp;
				}

		inline operator const char*() const
				{
					return m_pstrBuffer;
				}

		inline int GetLength() { return m_iActualSize; }

		ChBufferString& operator +=( char cChar );
		ChBufferString& operator +=( const char* pstrText );

		ChBufferString& Append( const char* pstrText, int iLen );
		ChBufferString& DeleteFromStart( int iLen );
		ChBufferString& Clear();

	protected:
		ChString		m_string;
		char*		m_pstrBuffer;
		char*		m_pstrBufferEnd;
		int			m_iBufferSize;
		int			m_iActualSize;
};


/*----------------------------------------------------------------------------
	ChAnsiState class
----------------------------------------------------------------------------*/

class ChAnsiState
{
	public:
		enum { constDefColor = 0xff };
		
		static const chuint32 colourTable[];

	public:
		ChAnsiState();
		virtual ~ChAnsiState() {}

		void Reset( ChString& strHTML );

		chuint8 GetForeColor()
					{
						return m_luForeColor;
					}	
		chuint8 GetBackColor()
					{
						return m_luBackColor;
					}
		bool GetBold()
					{
						return m_boolAnsiBold;
					}

		void SetBold( bool boolBold, ChString& strHTML );
		void SetUnderline( bool boolUnderline, ChString& strHTML );
		void SetItalic( bool boolItalic, ChString& strHTML );
		void SetInverse( bool boolInverse, ChString& strHTML );
		void SetStrikethrough( bool boolStrikethrough, ChString& strHTML );
		void SetForeColor( chuint8 luColor, ChString& strHTML );
		void SetBackColor( chuint8 luColor, ChString& strHTML );
		void SetColor( chuint8 luForeColor, chuint8 luBackColor,
						ChString& strHTML );
		
		void ChooseDefaultColors( ChString& strHTML );

	protected:
		bool		m_boolAnsiBold;
		bool		m_boolAnsiUnderline;
		bool		m_boolAnsiItalic;
		bool		m_boolAnsiInverse;
		bool		m_boolAnsiStrikethrough;
		bool		m_boolAnsiColor;
		chuint8	m_luForeColor;
		chuint8	m_luBackColor;
};


/*----------------------------------------------------------------------------
	ChWorldConn class
----------------------------------------------------------------------------*/

typedef enum { modeText, modeHtml, modePureHtml } TextMode;

enum { telnetStateEcho = 0x0001, telnetStateEOR = 0x0002, telnetStateTType = 0x0004,
				telnetStateMCCP1 = 0x0010, telnetStateMCCP2 = 0x0020, telnetStateMCCPOn = 0x0040 };

class ChWorldConn : public ChConn
{
	public:
		ChWorldConn( const ChModuleID& idModule,
						ChSocketHandler pHandler, ChWorldMainInfo* pMainInfo,
						chparam userData = 0 );
		virtual ~ChWorldConn();

		inline ChWorldMainInfo* GetMainInfo() const { return m_pMainInfo; }
		inline ChModuleID GetModuleID() const { return m_idModule; }
		inline ChWorldInfo* GetWorldInfo() { return m_pWorldInfo; }
		inline bool IsConnected() const { return stateConnected == m_state; }
		inline TextMode GetMode() const { return m_textMode; }
		inline bool IsPuebloEnhanced() const { return m_boolPuebloEnhanced; }
		inline void SendLine( const ChString& strLine )
						{
							ChConn::SendLine( (const char*)strLine );
						}
		inline void SetEchoPrefs( bool boolEcho, bool boolBold,
									bool boolItalic )
						{
							m_boolEcho = boolEcho;
							m_boolBold = boolBold;
							m_boolItalic = boolItalic;
						}
		inline void SetMode( TextMode mode )
						{
							m_textMode = mode;
						}

		inline void SetWorldInfo( ChWorldInfo* pWorldInfo )
						{
							m_pWorldInfo = pWorldInfo;
						}

		inline void TurnHtmlOn( ChString& strText )
						{
							if (GetMode() == modeText)
							{
								strText += MUD_TEXT_OFF;
							}
						}
		inline void TurnHtmlOff( ChString& strText )
						{
							if (GetMode() == modeText)
							{
								strText += MUD_TEXT_ON;
							}
						}

		void AsyncGetHostByName( const ChString& strHost,
									ChSocketAsyncHandler pprocHandler,
									chparam userData );
		
		void Connect( const ChString& strHost, chint16 sPort );
		void Connect( chuint32 luAddress, chint16 sPort );

		void ProcessOutput();

		void InitConnection();
		void TermConnection();

		void SendWorldCommand( const ChString& strCommand,
								bool boolUserCommand = true );
		void Display( const ChString& strText );
		void Display( ChString& strOut, const ChString& strText, bool boolUserCommand = false );

		void UpdatePreferences();

	protected:
		inline ChTextOutput* GetTextOutput() { return m_pTextOutput; }
		inline bool IsAnyTextReceived() { return m_boolTextReceived; }
		inline void SetAnyTextReceived() { m_boolTextReceived = true; }
		inline void SetPuebloEnhancedVersion( const ChVersion& versEnhanced )
						{
							m_versEnhanced = versEnhanced;
						}

		inline void OutputBuffer( const ChString& strBuffer,
									bool boolOutToDebug = true )
						{
							ASSERT( GetTextOutput() );

							GetTextOutput()->Add( strBuffer, boolOutToDebug );
						}

		inline void TelnetDisplay( const ChString& strSequence )
						{
							#if defined( DUMP_TELNET_CODES )
							{
								Display( strSequence );
							}
							#endif	// defined( DUMP_TELNET_CODES )
						}

		inline void AnsiDisplay( const ChString& strSequence )
						{
							#if defined( DUMP_ANSI_CODES )
							{
								GetMainInfo()->GetCore()->
									Trace( strSequence,
											ChCore::traceMiscMessages );
							}
							#endif	// defined( DUMP_ANSI_CODES )
						}

		void ParseIncomingData( ChString& strData, int iLen );

		void ParseText( ChString& strText, bool boolNewlinesToBreaks );
		void ParseEndOfLine( ChBufferString& strBuffer, int iStartOfLine,
								bool boolEndOfLine );

		void ProcessAnsiCodes( char cCodeType, const char* pstrCodes,
								ChBufferString& strBuffer );
		void ProcessAnsiCode( char cCodeType, int iCode, ChString& strOutput );

#if defined( DUMP_TELNET_CODES )
		ChString GetTelnetLabel( int iCommand );
#endif
		void TelnetSend( int iCommand, int iOption );
		void TelnetReceive( int iCommand, int iOption );
		void DoTelnetCmd( int iCommand, int iOption );
		void DoTelnetSubnegotiation( int iCommand, const ChString& data );
		void DoSendTerminalType();

		void TranslateNewlinesToBreaks( ChString& strOut );

		void AddToTextBlock( const char* pstrData, int iLen,
									bool boolEndOfLine );
		void OnTextLine( const ChString& strLine );
		
		void TextNotify();

		void SetPuebloEnhanced( bool boolEnhanced,
								const ChVersion& versEnhanced );


		void UpdateCompressionPanel();

	private:
		ChModuleID				m_idModule;
		ChWorldMainInfo*		m_pMainInfo;

		ConnectState			m_state;
		ChWorldInfo*			m_pWorldInfo;

		TextMode				m_textMode;
		bool					m_boolRawMode;
		bool					m_boolPuebloEnhanced;
		bool					m_boolTextReceived;
		ChVersion				m_versEnhanced;

		bool					m_boolEcho;
		bool					m_boolBold;
		bool					m_boolItalic;
		bool					m_boolAllowMCCP;

		ChAnsiState				m_ansiState;
		bool					m_boolAnsiResetReceived;

		ChTextOutput*			m_pTextOutput;

		chflag32				m_flTelnetState;
		ParseState				m_parseState;
		ParseState				m_telnetParseState;
		ChString					m_strAnsiSequence;
		int						m_iTelnetCmd;
		ChString					m_strCurrentLine;
		ChString					m_strSubnegotiation;
		
		int							m_iCurrentTermType;			// UE
		ChMCCP					m_mccp;
};


#endif	// !defined( _CHWCONN_H )

// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
// Revision 1.1.1.1  2003/02/03 18:53:20  uecasm
// Import of source tree as at version 2.53 release.
//
