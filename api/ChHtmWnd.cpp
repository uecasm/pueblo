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

	This file consists of the implementation of the ChHtmlWnd class.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <ctype.h>

#ifdef CH_UNIX
	#include <stdlib.h>
	#include <string.h>
	#include <malloc.h>
	#include <ctype.h>

	#include <ChTypes.h>
	#include <ChRect.h>
	#include <ChSize.h>
	#include <ChScrlVw.h>
	#include <ChDC.h>
#else
	#if !defined(CH_PUEBLO_PLUGIN)
	#include "resource.h"
	#else
	#include "vwrres.h"
	#endif
#endif

#include <iostream>
#include <fstream>

#include <ChConst.h>
#include "ChHTMLStream.h"
#include <ChHtpCon.h>
#include <ChHtmWnd.h>
#include <ChUtil.h>
#include <ChReg.h>
#include <ChHtmlSettings.h>

#include "ChHtmlView.h"
#include "ChHtmSym.h"
#include "ChHtmlParser.h"
#include "ChHtmlPane.h"

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
#endif

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

#include <MemDebug.h>



/*----------------------------------------------------------------------------
	Constants
------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
	Types
------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
	Variables
------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
	static member variables of ChHtmWnd
------------------------------------------------------------------------------*/

CH_INTERN_VAR ChString m_strHtmlClass;



/*----------------------------------------------------------------------------
	Utility functions
----------------------------------------------------------------------------*/

CH_INTERN_FUNC( void )
EscapeAmpersands( ChString& strText );

CH_INTERN_FUNC( void )
EscapeBrackets( ChString& strText );

/*----------------------------------------------------------------------------
	ChHtmlWnd constructor
---------------------------------------------------------------------------*/

#if !defined( CH_PUEBLO_PLUGIN )

ChHtmlWnd::ChHtmlWnd() : CWnd(),
			m_pHttpConnection( 0 )
{
 	InitHtmlWnd();
}

#endif


ChHtmlWnd::ChHtmlWnd( ChHTTPConn* pConn ) :
						CWnd(),
						m_pHttpConnection( pConn )
{
 	InitHtmlWnd();
}


void ChHtmlWnd::InitHtmlWnd()
{
	m_uPageID = 0;
	m_pRedirectStack = 0;
	m_pPaneMgr = 0;
	m_pSettings = new ChHtmlSettings();

	ASSERT( m_pSettings );
											/* Read preferences  for this HTML
												window */

	m_pSettings->SetProductName( CH_PRODUCT_NAME );

	if (0 == m_strHtmlClass.GetLength())
	{
		HBRUSH hbrBackground = CreateSolidBrush(m_pSettings->GetBackColor());	// UE
		
		m_strHtmlClass = AfxRegisterWndClass( CS_GLOBALCLASS,
												LoadCursor( 0, IDC_ARROW ),
												hbrBackground );
	}

											// HTTP Notification inits
	if (!m_pHttpConnection)
	{
		m_pHttpConnection = 0;
		m_phtmlStreamMgr = 0;
		m_boolMyConn = true;
	}
	else
	{
		m_boolMyConn = false;
		m_phtmlStreamMgr = new  ChHTMLStreamManager( this );
		ASSERT( m_phtmlStreamMgr );
	}
											/* Add a default view, update the
												view pointer later */

	ChHtmlView* pView = new ChHtmlView( TOP_WINDOW, this );
	ASSERT( pView );

	pView->SetEmbedMode( ChHtmlView::embedInternal );

 	ChHtmlViewObj * pFrameObj = new ChHtmlViewObj( 	ChString( TOP_WINDOW ), pView, (CFrameWnd*)0 );
	ASSERT( pFrameObj );
	m_htmlViewList.AddTail( pFrameObj );
}


/*----------------------------------------------------------------------------
	ChHtmlWnd destructor
---------------------------------------------------------------------------*/

ChHtmlWnd::~ChHtmlWnd()
{

	if ( m_boolMyConn )
	{
		// we use this for checking if the link has been visited
		delete m_pHttpConnection;
	}

	delete m_phtmlStreamMgr;

	delete m_pSettings;

	while ( m_pRedirectStack )
	{
		while( m_pRedirectStack->GetCount() )
		{
			delete  m_pRedirectStack->RemoveHead();
		}
		delete m_pRedirectStack;
		m_pRedirectStack = 0;		
	}
	

}

#if defined( CH_MSW )

BOOL ChHtmlWnd::Create( const CRect& rtView, CWnd* pParent, DWORD dwStyle,
						UINT uiID )
{
	dwStyle &= ~WS_VSCROLL;

	return CWnd::Create( m_strHtmlClass, "",  dwStyle, rtView, pParent, uiID );
}

BOOL ChHtmlWnd::CreateEx( const CRect& rtView, CWnd* pParent, DWORD dwStyle,
							DWORD dwStyleEx, UINT uiID )
{
	dwStyle &= ~WS_VSCROLL;

	return CWnd::CreateEx( dwStyleEx, m_strHtmlClass, "", dwStyle,  
							rtView.left, rtView.top, rtView.Width(), rtView.Height(),
							pParent->GetSafeHwnd(), (HMENU)uiID );
}

#endif	// defined( CH_MSW )


/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlWnd::NewPage

------------------------------------------------------------------------------
	Clear the contents of the current view and prepare for new data
----------------------------------------------------------------------------*/

bool ChHtmlWnd::NewPage( const char* pstrWindowName /* = 0 */ )
{

	m_uPageID++;

	if ( pstrWindowName )
	{
		ChHtmlView* pView = GetHtmlViewByName( pstrWindowName );
		if ( !pView )
		{
			return pView->NewPage( );
		}
		else
		{
			return false;
		}
	}
	else
	{
		CloseAllSubPanes();
	}
	return GetDefaultView()->NewPage();
}


void ChHtmlWnd::AppendText( const char* pstrNewText,
							chint32 lNewCount /* = -1 */,
							chflag32 flOptions /* = 0 */,
							const char* pstrWindowName /* = 0 */ )
{
	if (pstrWindowName)
	{
		ChHtmlView*		pView = GetHtmlViewByName( pstrWindowName );

		if (pView)
		{
			StreamText( pstrNewText, lNewCount, flOptions, pView );
		}
	}
	else if (m_pRedirectStack && m_pRedirectStack->GetCount())
	{
		ChRedirectInfo*	pRedirecInfo = m_pRedirectStack->GetHead();
		ChHtmlView*		pView = GetHtmlViewByName( pRedirecInfo->GetTarget() );

		if (pView)
		{
			StreamText( pstrNewText, lNewCount, flOptions, pView );
		}
		else
		{									// Skip till the next xch_pane
			chint32 lStart = 0;

			if (lNewCount == - 1)
			{
				lNewCount = lstrlen( pstrNewText );	
			}
			while (lStart < lNewCount)
			{
				if (pstrNewText[lStart] != TEXT( '<' ))
				{
					lStart++;
				}
				else
				{
					ChString		strToken;
					chint32		lTmp = lStart + 1;
					
					while (lTmp < lNewCount && !isspace( pstrNewText[lTmp] ))
					{
						strToken += pstrNewText[lTmp++];
					}

					if (strToken.CompareNoCase( TEXT( "xch_pane" ) ) == 0)
					{
						break;
					}
					else
					{
						lStart = lTmp;
					}
				}
			}

			if (lStart < lNewCount)
			{
				StreamText( pstrNewText, lNewCount, flOptions, GetDefaultView() );
			}
		}
	}
	else
	{
		StreamText( pstrNewText, lNewCount, flOptions, GetDefaultView() );
	}
}

//#define LOG_WINDOW

#if defined( LOG_WINDOW )
void WINAPI 
ChOutputToLog( const char* pstrBuffer, int iLen );
#endif


bool ChHtmlWnd::StreamText(	const char* pstrNewText, chint32 lNewCount,
						 chuint32 flOptions, ChHtmlView* pView )
{

	chint32 lStart = 0;

	if ( lNewCount == -1 )
	{
		lNewCount = lstrlen( pstrNewText );
	}

#if defined( LOG_WINDOW )
	ChOutputToLog( pstrNewText, lNewCount );
#endif

	while( lStart < lNewCount )
	{
		pView->AppendText( &pstrNewText[lStart], lNewCount - lStart, flOptions );

		// Check if we need to redirect to another stream 
		chint32 lStop;
		if ( pView->GetRedirectInfo( lStop ) )
		{

			lStart += lStop;

			if ( m_pRedirectStack && m_pRedirectStack->GetCount() )
			{
				ChRedirectInfo *pRedirecInfo = m_pRedirectStack->GetHead();

				ChHtmlView* pTmpView = GetHtmlViewByName( pRedirecInfo->GetTarget()  );	
				if ( !pTmpView )
				{
					// skip till the next xch_pane
					while( lStart < lNewCount )
					{
						if ( pstrNewText[lStart] != TEXT( '<' ) )
						{
							lStart++;
						}
						else
						{
							ChString strToken;
							chint32 lTmp = lStart+1;
							
							while( 	lTmp < 	lNewCount && !isspace( pstrNewText[lTmp] ) )
							{
								strToken += pstrNewText[lTmp++];	
							}
							if ( strToken.CompareNoCase( TEXT( "xch_pane" ) ) == 0 )
							{
								break;
							}
							else
							{
								lStart = lTmp;
							}
						}
					}
				}
				else
				{
				 	pView = pTmpView;
				}
			}
			else
			{
				return false;
			}
		}
		else
		{
			return true;
		}
	}

	return true;

}


bool ChHtmlWnd::StreamFile( const char* pstrFile, const char* pstrMimeType,
						chuint32 flOptions , const ChString& strURL, ChHtmlView* pView )
{

	ChString strDocMime( pstrMimeType ? pstrMimeType : "" );
	int iMimeType;

	if ( strDocMime.IsEmpty() )
	{
		ChHTTPConn::GetMimeTypeByFileExtn( pstrFile, strDocMime );
	}

	iMimeType = ChHTTPConn::GetMimeType( strDocMime );

	switch( iMimeType )
	{
		case ChHTTPConn::typeHTML:
		case ChHTTPConn::typeText:
		{
			break;
		}
		default :
		{
			return pView->DisplayFile( pstrFile, strDocMime, flOptions, strURL );
		}
	}
	// We need to stream

	//#ifdef CH_MSW
//		fstream *pFile = ::new fstream( pstrFile, ios::in | ios::nocreate , filebuf::sh_read );
		std::fstream *pFile = ::new std::fstream( pstrFile, std::ios::in );
	//#else
	//	fstream *pFile = ::new fstream( pstrFile, ios::in | ios::nocreate );
	//#endif

	ASSERT( pFile );

	if ( !pFile->is_open() )
	{
		::delete pFile;
		return false;
	}

	if ( flOptions & ChHtmlWnd::fileReplace )
	{
		pView->NewPage();
	}

// - removed: this doesn't compile in BC++, and files default to text mode anyway
//	#if defined( CH_MSW )
//	{										// set the file read to text mode
//		pFile->setmode( filebuf::text );
//	}
//	#endif

	pView->SetDocURL( strURL );
	pView->SetDocBaseURL( strURL );		// UE: set as base URL for page, for further relative links etc

	char *pstrBuffer = new char[ 4092 ];
	ASSERT( pstrBuffer );

	// parse the HTML text file
	while( ( pFile->read( pstrBuffer, 4092 ).gcount()) )
	{

		StreamText( pstrBuffer, pFile->gcount(), flOptions, pView );

		// Pump messages 
		{
			MSG msg;
			int iMaxMsgs = 0;
			CWinThread* pThread = AfxGetThread();

			while ( !IsAborted() && 
						pThread && iMaxMsgs++ < 20 && 
						::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE ) )
			{
				::DispatchMessage(&msg);
				// allow user-interface updates
				pThread->OnIdle(-1);
			}
		}

	}

	pFile->close();

	::delete pFile;
	delete []pstrBuffer;

	return true;
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlWnd::DisplayFile

------------------------------------------------------------------------------
	This method takes a text file and displays the file.
----------------------------------------------------------------------------*/

bool ChHtmlWnd::DisplayFile( const char* pstrFile,	const char* pstrMimeType /* = 0 */,
						chuint32 flOptions /*= ChHtmlWnd::fileReplace */,
						ChString strURL /* = "" */,
						const char* pstrWindowName /* = 0 */)
{


	ChHtmlView*  pView = GetDefaultView();;	
	ChHtmlView*  pDefView = pView;
		
	if ( pstrWindowName )
	{
		pView = GetHtmlViewByName( pstrWindowName );	
		if ( pView != pDefView )
		{
			
			return StreamFile( pstrFile, pstrMimeType, flOptions, strURL, pView );
		}
	}

		
	if ( pView == pDefView )
	{
		NewPage( pstrWindowName );
		while ( m_pRedirectStack )
		{
			while( m_pRedirectStack->GetCount() )
			{
				delete  m_pRedirectStack->RemoveHead();
			}
			delete m_pRedirectStack;
			m_pRedirectStack = 0;		
		}
		// Close all connection for the previous page
		GetHTTPConn()->AbortRequests( false, m_phtmlStreamMgr );

		return StreamFile( pstrFile, pstrMimeType, flOptions, strURL, pDefView );
	}
	else
	{
		return false;
	}
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlWnd::DisplayResource

------------------------------------------------------------------------------
	This method takes a resource block of type "HTML" and displays the
	data from that resource.  It's assumed that the resource is a zero-
	terminated text block.
----------------------------------------------------------------------------*/

void ChHtmlWnd::DisplayResource( int iResID, chuint32 flOptions,
									HINSTANCE hModule,
									const char* pstrWindowName /* = 0 */ )
{
	if ( pstrWindowName )
	{
		ChHtmlView* pView = GetHtmlViewByName( pstrWindowName );	
		if ( pView )
		{
			pView->DisplayResource( iResID, flOptions, hModule );
		}
		else
		{
			return;
		}
	}
	NewPage();
	GetDefaultView()->DisplayResource( iResID, flOptions, hModule );
}

/*----------------------------------------------------------------------------
	The next three functions will parse a specified string, escaping any
	ampersands it finds to '&&;' and any '<' characters to '&LT;'.
----------------------------------------------------------------------------*/

void ChHtmlWnd::EscapeForHTML( ChString& strTextOut )
{
	int	 	iIndex = 0;
	int 	iLen = strTextOut.GetLength();
	int 	iExtra = 0;


	char * pstrBuffer = strTextOut.GetBuffer( iLen );

	while ( pstrBuffer[iIndex] )
	{
		if ( TEXT( '&') == pstrBuffer[iIndex] )
		{
			if ( iExtra < 4 )
			{
				iExtra += 64;
				strTextOut.ReleaseBuffer();
				pstrBuffer = strTextOut.GetBuffer( iLen +iExtra );
			}

			iIndex++;
			ChMemMove( &pstrBuffer[iIndex + 4 ], &pstrBuffer[iIndex], iLen - iIndex + 1 );
			ChMemMove( &pstrBuffer[iIndex], TEXT( "amp;" ), 4 * sizeof(TCHAR) );

			iLen 	+= 4;
			iExtra  -= 4;
			iIndex  += 4;

		}
		else if ( TEXT( '<') == pstrBuffer[iIndex] )
		{
			if ( iExtra < 4 )
			{
				iExtra += 64;
				strTextOut.ReleaseBuffer();
				pstrBuffer = strTextOut.GetBuffer( iLen +iExtra );
			}


			ChMemMove( &pstrBuffer[iIndex + 4 ], &pstrBuffer[iIndex + 1], iLen - iIndex );
			ChMemMove( &pstrBuffer[iIndex], TEXT( "&lt;" ), 4 * sizeof(TCHAR) );
			iLen += 4;
			iExtra -= 4;
 			iIndex  += 4;

		}
		else
		{
			iIndex++;
		}
	}

	strTextOut.ReleaseBuffer( iIndex );

}

#if 0
CH_INTERN_FUNC( void )
EscapeAmpersands( ChString& strText )
{
	if (-1 != strText.Find( '&' ))
	{										/* Do this work if there are
												ampersands in the string */
		int		iLoop;
		ChString	strOut;

		for (iLoop = 0; iLoop < strText.GetLength(); iLoop++)
		{
			register char	cCurr = strText[iLoop];

			if ('&' == cCurr)
			{								// Replace '&' with '&&;'
				strOut += "&&;";
			}
			else
			{
				strOut += cCurr;
			}
		}

		strText = strOut;
	}
}


CH_INTERN_FUNC( void )
EscapeBrackets( ChString& strText )
{
	if (-1 != strText.Find( '<' ))
	{										/* Do this work if there are
												ampersands in the string */
		ChString	strOut;
		int		iLoop;

		for (iLoop = 0; iLoop < strText.GetLength(); iLoop++)
		{
			register char	cCurr = strText[iLoop];

			if ('<' == cCurr)
			{								// Replace '<' with '&LT;'
				strOut += "&LT;";
			}
			else
			{
				strOut += cCurr;
			}
		}

		strText = strOut;
	}
}    
#endif                                                                                                                                                                                      


/*----------------------------------------------------------------------------
	This function will parse the HTML argument string, looking for a
	specified attribute label and returning the corresponding argument.

	If the attribute label is found the function returns true and any
	corresponding value.  If the attribute label is not found, the function
	returns false and sets the value to an empty string.
----------------------------------------------------------------------------*/

bool ChHtmlWnd::GetHTMLAttribute( const char* pstrArgs,
									const char* pstrType,
									ChString& strVal )
{
	chint32		iIndex = 0;

	while( true )
	{
											// Remove any leading white space
		while (isspace( pstrArgs[iIndex] ))
		{
			++iIndex;
		}
											// End of arg list ?

		if ((pstrArgs[iIndex] == TEXT( '>' )) || (0 == pstrArgs[iIndex]))
		{
			break;
		}
											// Get the argument	type
		ChString		strAttr;
  		int			iAttrLen = 0;
											// force padding to concat faster
		char* 		pstrAttr = strAttr.GetBuffer(lstrlen(pstrArgs)); 	

		while (pstrArgs[iIndex] &&
				!isspace( pstrArgs[iIndex] ) &&
				(pstrArgs[iIndex] != TEXT( '=' )) &&
				(pstrArgs[iIndex] != TEXT( '>' )))
		{
			pstrAttr[iAttrLen++] = pstrArgs[iIndex++];
		}

  		// Resize the buffer
  		strAttr.ReleaseBuffer( iAttrLen );

		while (pstrArgs[iIndex] && isspace( pstrArgs[iIndex] ))
		{
			++iIndex;
		}
											// Is it the one we want?
		if ( strAttr.CompareNoCase( pstrType ) == 0 )
		{									// YES!

			int		iValIndex = 0;
											// force padding to concat faster
			char* pstrVal = strVal.GetBuffer(lstrlen(&pstrArgs[iIndex]));	

			if (pstrArgs[iIndex] == TEXT( '=' ))
			{
											/* We have a attribute... remove
												any leading white space */
				iIndex++;

				while (isspace( pstrArgs[iIndex] ))
				{
					++iIndex;
				}

											// Get the value
				bool	boolSpaceTerminated;

				if ( CHAR_DBL_QUOTE == pstrArgs[iIndex] || CHAR_SGL_QUOTE == pstrArgs[iIndex] )
				{
					boolSpaceTerminated = false;
				}
				else
				{
					boolSpaceTerminated = true;
				}

				if (boolSpaceTerminated)
				{
					while (pstrArgs[iIndex] &&
							!isspace( pstrArgs[iIndex] ) &&
							(pstrArgs[iIndex] != TEXT( '>' )))
					{
						char strChar = pstrArgs[iIndex++];
						if ( strChar == TEXT( '&' ) )
						{
							--iIndex;
							strChar = ChHtmlParser::MapEntity( pstrArgs, 
										iIndex, lstrlen(pstrArgs) );
						}
						pstrVal[iValIndex++] = strChar;
					}
				}
				else
				{							// Quoted value

					char	strTerm;

					strTerm	= pstrArgs[iIndex];
					iIndex++;

					while (pstrArgs[iIndex] &&
							(pstrArgs[iIndex] != strTerm ))
					{

						char strChar = pstrArgs[iIndex++];
						if ( strChar == TEXT( '&' ) )
						{
							--iIndex;
							strChar = ChHtmlParser::MapEntity( pstrArgs, 
										iIndex, lstrlen(pstrArgs) );
						}
						else if ( IS_WHITE_SPACE( strChar ) )
						{
							strChar = TEXT( ' ' );
						}
						pstrVal[iValIndex++] = strChar;
					}
				}
			}
										// Found the attr / value pair
			strVal.ReleaseBuffer( iValIndex );
			return true;
		}
		else
		{									/* This is the wrong attribute.
												Skip any value, ignoring
												all white space */

			while (pstrArgs[iIndex] && isspace( pstrArgs[iIndex] ))
			{
				++iIndex;
			}

			if ('=' == pstrArgs[iIndex])
			{
				++iIndex;

				while (pstrArgs[iIndex] && isspace( pstrArgs[iIndex] ))
				{
					++iIndex;
				}
												// Skip the value
				bool	boolSpaceTerminated;

				if ( TEXT( '"' ) == pstrArgs[iIndex])
				{
					boolSpaceTerminated = false;
					iIndex++;
				}
				else
				{
					boolSpaceTerminated = true;
				}

				if (boolSpaceTerminated)
				{
					while (pstrArgs[iIndex] &&
							!isspace( pstrArgs[iIndex] ) &&
							(pstrArgs[iIndex] != TEXT( '>' )))
					{
						iIndex++;
					}
				}
				else
				{
					while (pstrArgs[iIndex] &&
							(pstrArgs[iIndex] != TEXT( '"' )) &&
							(pstrArgs[iIndex] != TEXT( '>' )))
					{
						iIndex++;
					}

					if (pstrArgs[iIndex])
					{
						iIndex++;
					}
				}
			}
		}
	}

	return false;
}


/*----------------------------------------------------------------------------
	This function will search a HTML string for a HREF attribute.  It is
	a shortcut to the GetHTMLAttribute method.

	If boolImage is true and the HREF attribute is not found, then a
	SRC attribute will also be searched for.
----------------------------------------------------------------------------*/

bool ChHtmlWnd::GetHTMLHref( const char* pstrArgs, bool boolImage,
								ChString& strVal )
{
	bool	boolFound;

	boolFound = GetHTMLAttribute( pstrArgs, ATTR_HREF, strVal );

	if (boolImage && !boolFound)
	{
		boolFound = GetHTMLAttribute( pstrArgs, ATTR_SRC, strVal );
	}

	return boolFound;
}


void ChHtmlWnd::RemoveEntities( const char* pstrBuffer, ChString& strResult )
{
	int		iLen = strlen( pstrBuffer );
	ChString	strOutbuffer;

	LPTSTR 	pstrOutBuffer = strOutbuffer.GetBufferSetLength( iLen + 1 );
	int		iOut = 0;

	while (iLen)
	{
		if ('&' == *pstrBuffer)
		{
			chint32		lStart = 0;

			pstrOutBuffer[iOut++] = ChHtmlParser::MapEntity( pstrBuffer, lStart, iLen );

			pstrBuffer += lStart;
			iLen -= lStart;
		}
		else
		{
			pstrOutBuffer[iOut++] = *pstrBuffer++;
			iLen--;
		}
	}

	strOutbuffer.ReleaseBuffer();

	strResult = strOutbuffer;
}

void ChHtmlWnd::OnViewHotSpot( ChHtmlView* pView, chparam userData )
{
	OnHotSpot( userData, pView->GetDocURL()  );
}

void ChHtmlWnd::OnViewSelectHotSpot( ChHtmlView* pView, int x, int y, ChPoint& ptRel, 
								chparam userData )
{
	if ( !OnSelectHotSpot( x, y, ptRel, userData,	pView->GetDocURL() ) )
	{
		if (userData)
		{
			char*	pstrCmd = (char*)userData;
			ChString	strURL;

			if (GetHTMLHref( pstrCmd, false, strURL ))
			{


				TRACE1( "HTML Hot spot:%s\n", pstrCmd );
				if ( ptRel.x != -1 && ptRel.y != -1 )
				{
					ChString  strCoord;

					strCoord.Format( "?%d,%d", ptRel.x, ptRel.y );

					strURL += strCoord;
	 				TRACE1( "HTML ISMAP Hot spot:%s\n", LPCSTR(strURL) );

				}
				ChString strTarget;
				if ( GetHTMLAttribute( pstrCmd,	ATTR_TARGET, strTarget ) )
				{
					strTarget.MakeLower();
					if ( strTarget == SELF_WINDOW || strTarget == CURRENT_WINDOW )
					{
						strTarget = pView->GetFrameName();	
					}
				}
				else
				{
					strTarget = TOP_WINDOW;	
				}

				ChURLParts  urlParts;

				urlParts.GetURLParts( strURL, pView->GetDocBaseURL() );
				LoadURL( urlParts.GetURL(), strTarget );
			}
		}
	}
}

// this method is called when the HTML parser detects an inline image
// in the the document, the data passed to the method is the arguments associated with
// with the tag buffer

bool ChHtmlWnd::OnViewInline( ChHtmlView* pView, const char* pstrArgs )
{
	return OnInline( pstrArgs, pView->GetDocURL()  );
}

// called when user submits a form with a non-standard method type.
void ChHtmlWnd::OnViewSubmitForm( ChHtmlView* pView, const ChString& strAction, 
								const ChString& strMD5, const ChString& strFormData )
{
	OnSubmitForm( strAction, strMD5, strFormData );
}


/*----------------------------------------------------------------------------
	Override this virtual member function if you need notification when
	the user clicks on a hotspot.
----------------------------------------------------------------------------*/

bool ChHtmlWnd::OnSelectHotSpot( int x, int y, ChPoint& ptRel, chparam userData,
									const ChString& strDocURL )
{
	return false;
}


/*----------------------------------------------------------------------------
	Override this virtual member function if you need notification when
	the mouse is on a hotspot.
----------------------------------------------------------------------------*/

void ChHtmlWnd::OnHotSpot( chparam userData, const ChString& strDocURL  )
{
}


/*----------------------------------------------------------------------------
	Override this virtual member function if you need notification when
	there is a redirect of URL to external browser
----------------------------------------------------------------------------*/

bool ChHtmlWnd::OnRedirect(  const ChString& strURL, bool boolWebTracker  )
{
	return false;
}


/*----------------------------------------------------------------------------
	Override this virtual member function if you need notification when
	the HTML parser detects an inline image in the document.
----------------------------------------------------------------------------*/

bool ChHtmlWnd::OnInline( const char* pstrArgs, const ChString& strDocURL  )
{
	return false;
}


/*----------------------------------------------------------------------------
	Override this virtual member function if you need notification when
	user submits a form with a method which is not of type Post or Get.
----------------------------------------------------------------------------*/

void ChHtmlWnd::OnSubmitForm( const ChString& strCommand, const ChString& strMD5,
												const ChString& strFormData )
{
}



/*----------------------------------------------------------------------------
	Override this virtual member function if you need to do something special
	when a load complete occurs. Calling the base class Displays the file
----------------------------------------------------------------------------*/

void ChHtmlWnd::OnLoadComplete( const ChString& strFile, const ChString& strURL,
									const ChString& strMimeType, chparam userData )
{	

	ChHtmlURLReq *pReq = (ChHtmlURLReq*)userData;
	if( !DisplayFile( strFile,	strMimeType, ChHtmlWnd::fileReplace, 
					strURL, pReq->GetTarget() ) )
	{	// Load error
		
	}
}


/*----------------------------------------------------------------------------
	Override this virtual member function if you need to do something special
	when a load error occurs. 
----------------------------------------------------------------------------*/

void ChHtmlWnd::OnLoadError( chint32 lError, const ChString& strErrMsg, const ChString& strURL,
								chparam userData )
{	
}


/*----------------------------------------------------------------------------
	Override this virtual member function to display trace messages generated
	by HTML parser, HTTP load errors, plugin errors etc.
----------------------------------------------------------------------------*/
void ChHtmlWnd::OnTrace( const ChString& strMsg, int iType )
{

}


////////////////////////////////////////////////////////////////////////////////////


// HTML Wnd native message handlers

#ifdef CH_MSW
BEGIN_MESSAGE_MAP(ChHtmlWnd, CWnd)
	//{{AFX_MSG_MAP(ChHtmlWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_NCLBUTTONUP()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
	ON_MESSAGE( WM_HTML_FONT_CHANGE, OnFontChange )
	ON_MESSAGE( WM_HTML_COLOR_CHANGE, OnColorChange )
	ON_MESSAGE( WM_CHACO_HTTP_MSG, OnHTTPNotificaton )
	ON_MESSAGE( WM_CTLCOLORDLG, OnCtlBkColor )
END_MESSAGE_MAP()
#endif 


/*----------------------------------------------------------------------------
	ChHtmlWnd class message handlers
----------------------------------------------------------------------------*/

#ifdef CH_UNIX
chuint32 ChHtmlWnd::OnFontChange( chuint32 wParam, chuint32 lParam )
#else
LONG ChHtmlWnd::OnFontChange( UINT wParam, LONG lParam )
#endif
{

	ChString		strOldFixed =  m_pSettings->GetFixedFontName();
	ChString		strOldProportional = m_pSettings->GetProportionalFontName();
	int			iOldFixedSize = m_pSettings->GetFixedFontSize();
	int			iOldProportionalSize = m_pSettings->GetProportionalFontSize();



	// Read the new settings
	m_pSettings->ReadPreferences();

	if ( m_htmlViewList.GetCount())
	{

		ChPosition pos = m_htmlViewList.GetHeadPosition();

		while( pos != 0 )
		{
			ChHtmlViewObj*	pFrameObj = m_htmlViewList.GetNext( pos );

			pFrameObj->GetHtmlView()->UpdateFontChange( strOldProportional, iOldProportionalSize, 
												strOldFixed, iOldFixedSize );
		}
	}

	return 0;
}


#ifdef CH_UNIX
chuint32 ChHtmlWnd::OnColorChange( chuint32 wParam, chuint32 lParam )
#else
LONG ChHtmlWnd::OnColorChange( UINT wParam, LONG lParam )
#endif
{
	chuint32   luOldTextColor, luOldLinkColor, 
				luOldVLinkColor, luOldPrefetchColor, luBackColor;

	luOldTextColor = m_pSettings->GetTextColor();
	luOldLinkColor = m_pSettings->GetLinkColor();
	luOldVLinkColor = m_pSettings->GetVistedLinkColor();
	luOldPrefetchColor = m_pSettings->GetPrefetchedLinkColor();
	luBackColor = m_pSettings->GetBackColor();

		// Read the new settings
	m_pSettings->ReadPreferences();

	// free all the forms
	if ( m_htmlViewList.GetCount())
	{

		ChPosition pos = m_htmlViewList.GetHeadPosition();

		while( pos != 0 )
		{
			ChHtmlViewObj*	pFrameObj = m_htmlViewList.GetNext( pos );

			pFrameObj->GetHtmlView()->UpdateColorChange( luOldTextColor, luOldLinkColor, 
												luOldVLinkColor, luOldPrefetchColor, 
												luBackColor );


		}
	}

	return 0;
}

LONG ChHtmlWnd::OnCtlBkColor( WPARAM wParam, LPARAM lParam )
{
	return GetParent()->SendMessage( WM_CTLCOLORDLG, wParam, lParam );
}


int ChHtmlWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}
											// Create the default HTML view
	if (CreateHtmlView( TOP_WINDOW ))
	{
		RecalcLayout();
		return 0;
	}
	
	return -1;
}


void ChHtmlWnd::OnSize( UINT nType, int cx, int cy )
{
	CWnd::OnSize( nType, cx, cy );
	
	RecalcLayout();
}

void ChHtmlWnd::OnSetFocus(CWnd* pOldWnd) 
{
	CWnd::OnSetFocus(pOldWnd);

	if ( m_htmlViewList.GetCount() )
	{
		if ( ::IsWindow( GetDefaultView()->GetSafeHwnd() ) )
		{
			GetDefaultView()->SetFocus();
		}
	}
}


void ChHtmlWnd::OnNcLButtonUp( UINT nHitTest, CPoint point )
{
	CWnd::OnNcLButtonUp( nHitTest, point );

	OnMouseUp( );
}


void ChHtmlWnd::OnDestroy() 
{
	CWnd::OnDestroy();
											/* Close all outstanding
												connections for the
												current page */
	m_uPageID++;

	if ( GetHTTPConn() )
	{
		GetHTTPConn()->AbortRequests( true, m_phtmlStreamMgr );
	}

	if (m_htmlViewList.GetCount())
	{
		ChPosition	pos = m_htmlViewList.GetHeadPosition();

		while (pos != 0)
		{
			ChPosition		posDel = pos;
			ChHtmlViewObj*	pFrameObj = m_htmlViewList.GetNext( pos );
	
			m_htmlViewList.Remove( posDel );
			delete pFrameObj;

			pos = m_htmlViewList.GetHeadPosition();
		}
	}
}



// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
// Revision 1.1.1.1  2003/02/03 18:54:17  uecasm
// Import of source tree as at version 2.53 release.
//
