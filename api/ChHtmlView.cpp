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

	This file consists of the implementation of the ChHtmlView class.

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

#include <ChUtil.h>
#include <ChHTTP.h>

#include <ChHtmWnd.h>
#include <ChHtmlSettings.h>

#include "ChHtmlStream.h"
#include "ChHtmlView.h"
#include "ChPlgInMgr.h"
#include "ChHtmlPane.h"

#include "ChHtmSym.h"
#include "ChHtmlParser.h"

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


class ZapInlineData : public ChVisitor2<ChString, pChInlineImageData >  
{
	public:
	 	bool Visit( const ChString& key,  const pChInlineImageData& pInline )
		{
			delete pInline;
			return true;
		}
};


/*----------------------------------------------------------------------------
	Variables
------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
	static member variables of ChHtmWnd
------------------------------------------------------------------------------*/

HCURSOR		ChHtmlView::m_hHTMLCursors[maxCursors];		// hotspot cursor


/*----------------------------------------------------------------------------
	ChHTMLPrefetch 
---------------------------------------------------------------------------*/

ChHTMLPrefetch::ChHTMLPrefetch() : m_pstrArg(0), 
				m_pstrhRef(0), m_iProbablity(0)
{
}


ChHTMLPrefetch::~ChHTMLPrefetch()
{
	if ( m_pstrArg )
	{
		delete[] m_pstrArg;
	}
}



/*----------------------------------------------------------------------------
	ChHtmlView constructor
---------------------------------------------------------------------------*/

ChHtmlView::ChHtmlView( const char* pstrName, ChHtmlWnd* pFrameMgr ) : ChTxtWnd(),
							 m_pFrameMgr( pFrameMgr ),
							 m_pParser( 0 )
{
	m_strFrameName = pstrName;
 	InitHtmlView();
}


void ChHtmlView::InitHtmlView()
{
	m_boolAbortFormatting = false;
	m_boolFormatting = false;
	m_uPageNumber  = 0;
	m_iEmbedMode = 0;

	// plugin manager
	m_pluginMgr = 0;

	// Do the static initialization
	ChPlugInMgr::Initialize();
	
	m_strTargetWindowName.Empty();

	// Set the current state to paragragh
	m_hCursor = NULL;

	if ( !ChTxtWnd::m_defBkColor.GetSafeHandle() )
	{   // First time, create the brush
	 	ChTxtWnd::m_defBkColor.CreateSolidBrush( GetSettings()->GetBackColor() );
	}
	
	// UE: I really wanted to change this, to get rid of the strange gutter at the
	//     left margin.  Unfortunately, it causes too many annoying side effects.
	m_pageIndent.SetRect( GetSettings()->GetCharWidth(), 0, 0, 0 );
  SetViewIndents( m_pageIndent );

	#if defined( CH_MSW )
	if ( 0 == m_hHTMLCursors[ cursorNormal ] )
	{
		// load all HTML cursors

		m_hHTMLCursors[ cursorNormal ] = ::LoadCursor( NULL, IDC_ARROW	);
		m_hHTMLCursors[ cursorText ] = ::LoadCursor( NULL, IDC_IBEAM);
		#if defined( CH_PUEBLO_PLUGIN )
		m_hHTMLCursors[ cursorHotspot ] = ::LoadCursor( AfxGetInstanceHandle(),
									MAKEINTRESOURCE( IDC_HOTSPOT_CUR ) );
		#else
		m_hHTMLCursors[ cursorHotspot ] = ::LoadCursor( PuebloDLL.hModule,
									MAKEINTRESOURCE( IDC_HOTSPOT_CUR ) );
		#endif
	}
	#elif defined( CH_UNIX )
	{
		cerr << "XXX Not setting cursor." << __FILE__ << ":" << __LINE__ << endl;
	}
	#endif	// defined( CH_UNIX )


	if ( !m_pParser )
	{
		m_pParser = new ChHtmlParser( this );
		ASSERT( m_pParser );
		m_boolDeleteParser = true;
	}
	else
	{
		m_boolDeleteParser = false;
	}


	// form list
	m_pformLst = NULL;
}


/*----------------------------------------------------------------------------
	ChHtmlView destructor
---------------------------------------------------------------------------*/

ChHtmlView::~ChHtmlView()
{

	// cleanup the plugin manager
	delete m_pluginMgr;


	// free all the forms
	if ( m_pformLst && m_pformLst->GetCount())
	{

		ChPosition pos = m_pformLst->GetHeadPosition();

		while( pos != 0 )
		{
			ChPosition	prevPos = pos;
			ChHTMLForm*	pForm = m_pformLst->GetNext( pos );

			m_pformLst->Remove( prevPos );
			delete pForm;

			pos = m_pformLst->GetHeadPosition();
		}
	}
	// delete the form list
	delete m_pformLst;


	if (m_cmdList.GetCount())
	{

		ChPosition pos = m_cmdList.GetHeadPosition();

		while( pos != 0 )
		{
			ChPosition	prevPos = pos;
			char*	pCmd = m_cmdList.GetNext( pos );

			m_cmdList.Remove( prevPos );
			delete []pCmd;

			pos = m_cmdList.GetHeadPosition();
		}
	}

	// Delete all the image 
	{ 
		ZapInlineData		zapImage;

	  	GetImageList().Infix( zapImage );

		GetImageList().Erase();
	}

	
	if ( m_boolDeleteParser )
	{ // this is mine
		delete m_pParser;
	}

											// Remove any old bkcolor
	if (m_bodyBkColor.GetSafeHandle())
	{
		m_bodyBkColor.DeleteObject();
	}
	delete m_pbkImage;
	m_pbkImage = 0;
	delete m_pbackGround;
	m_pbackGround = 0;

}

#if defined( CH_MSW )

BOOL ChHtmlView::Create( const CRect& rtView, CWnd* pParent, DWORD dwStyle,
						UINT uiID )
{
	return ChTxtWnd::Create( "", dwStyle, rtView, pParent, uiID );
}

BOOL ChHtmlView::CreateEx( const CRect& rtView, CWnd* pParent, DWORD dwStyle,
							DWORD dwStyleEx, UINT uiID )
{
	return ChTxtWnd::CreateEx( "", dwStyle, dwStyleEx, rtView, pParent, uiID );
}

#endif	// defined( CH_MSW )


void ChHtmlView::GetIdealSize( ChSize& size )
{
	GetDocumentSize( size );
}


void ChHtmlView::OnFrameDisconnect( const ChModuleID& idNewModule )
{
	GetFrameMgr()->ClosePane( GetFrameName(), false );
	DestroyWindow();
	//delete this;	// UE: methinks this is already being done elsewhere
}


void ChHtmlView::OnMouseUp()
{
	if ( m_iEmbedMode == embedInternal )
	{
		GetFrameMgr()->OnMouseUp();
	}
}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlView::NewPage

------------------------------------------------------------------------------
	Clear the contents of the current view and prepare for new data
----------------------------------------------------------------------------*/

bool ChHtmlView::NewPage()
{
	// New page ID, this is used to determine if we are in the
	// right page on LoadComplete 
	// There should be no formatting in progress when we are here
	ASSERT( !m_boolFormatting );

	m_boolAbortFormatting = false;	

	m_uPageNumber++;

	m_strDocURL.Empty();
	m_strDocBaseURL.Empty();
	m_strDocTitle.Empty();


	// free all the resources allocated for
	// hotspots
	if (m_cmdList.GetCount())
	{

		ChPosition pos = m_cmdList.GetHeadPosition();

		while (pos != 0)
		{
			ChPosition	prevPos = pos;
			char*	pCmd = m_cmdList.GetNext( pos );

			m_cmdList.Remove( prevPos );
			delete []pCmd;

			pos = m_cmdList.GetHeadPosition();
		}
	}

	// free all the forms
	if ( m_pformLst && m_pformLst->GetCount())
	{

		ChPosition pos = m_pformLst->GetHeadPosition();

		while( pos != 0 )
		{
			ChPosition	prevPos = pos;
			ChHTMLForm*	pForm = m_pformLst->GetNext( pos );

			m_pformLst->Remove( prevPos );
			delete pForm;

			pos = m_pformLst->GetHeadPosition();
		}
	}
											// Delete the contents of the view
	SetUpdateIndex( 0 ); // Start update from the top of the page

	#if 0
	ChTxtWnd::DeleteText( 0, ChTxtWnd::GetTextCount() );
	#endif
	ChTxtWnd::ClearPage();


											// Delete all the image 
	{ 
		ZapInlineData		zapImage;

	  	GetImageList().Infix( zapImage );

		GetImageList().Erase();
	}

											// Rest the scroll bar
	SetScrollPos( SB_VERT, 0 );
											/* Clear any document attrs for
												vertical center if any */

	SetDocumentAttrs( (GetDocumentAttrs() & ~(ChTxtWnd::docVCenter)) );

	#if 0
	#if defined( CH_MSW )
	{
		::SetCursor( ::LoadCursor( 0, IDC_ARROW ) );

											// Remove any old bkcolor
		if (m_bodyBkColor.GetSafeHandle())
		{
			m_bodyBkColor.DeleteObject();
		}
											// Delete the background
		delete m_pbkImage;
		m_pbkImage = 0;

		delete m_pbackGround;
		m_pbackGround = 0;
	}
	#endif	// defined( CH_MSW )
	#endif
											/* This can be overridden by the
												<body> tag */
   
	SetViewIndents( m_pageIndent );

											// Turn it on just in case !!!
	SetRedraw();
											// Clear the view
	InvalidateRect( 0, true );		 
											// Initialize the parser
	m_pParser->InitParser();

	return true;
}


void ChHtmlView::AppendText( const char* pstrNewText, chint32 lNewCount, chuint32 flOptions )
{


	SetUpdateIndex( ChTxtWnd::GetTextCount() );
	bool	boolDisplayAppend = CalcDisplayAppend();

	// if logging is on then write to log file
	if ( LogHTML() )
	{
		LogToFile( pstrNewText, lNewCount );
	}

	if ( flOptions & ChHtmlWnd::modeUntranslated )
	{
		TRACE1("AppendUntranslated: \"%s\"\n", pstrNewText);
		AppendUntranslated( pstrNewText, lNewCount );
		// did we add anything to the view ?
		if ( ChTxtWnd::GetTextCount() != GetUpdateIndex() )
		{
			// commit all the changes made to text view, this causes it to be render
			ChTxtWnd::UpdateAppendText( GetUpdateIndex(), boolDisplayAppend );
		}

	}
	else
	{

		// Assume that we there will be no redirection of stream
		SetRedirectInfo( false, lNewCount );


		m_pParser->ParseText( pstrNewText, lNewCount );

		// did we add anything to the view ?
		if ( ChTxtWnd::GetTextCount() != GetUpdateIndex() )
		{
			// commit all the changes made to text view, this causes it to be render
			ChTxtWnd::UpdateAppendText( GetUpdateIndex(), boolDisplayAppend );
		}
		// if there was redirection, check if this is a size to fit pane, if it is then
		// recalc layout
		if ( m_boolRedirect && m_pFrameMgr->GetPaneMgr() )
		{
			ChPane*		pPane; 

			pPane = m_pFrameMgr->GetPaneMgr()->FindPane( GetFrameName() );
			if (pPane && pPane->IsSizeToFit() 
					&& GetViewHeight() != GetCanvasHeight() )
			{
								/* New data has been added recalc layout*/

				m_pFrameMgr->GetPaneMgr()->RecalcLayout( pPane );
			}
		}

		// if this append  had any prefetch tags, call the virtual function
		if (m_prefetchList.GetCount())
		{

			ChPosition pos = m_prefetchList.GetHeadPosition();

			while( pos != 0 )
			{
				ChPosition	prevPos = pos;
				ChHTMLPrefetch*	pFetch = m_prefetchList.GetNext( pos );

				m_prefetchList.Remove( prevPos );
												// Notify the prefetch
				OnPrefetch( pFetch, GetDocBaseURL() );

				delete pFetch;

				pos = m_prefetchList.GetHeadPosition();
			}
		}
	}

} 

void ChHtmlView::AppendUntranslated( const char* pstrNewText, chint32 lNewCount )
{
	ChFont*	 pOldFont   = 	m_pParser->GetTextStyle()->GetFont();
	chuint32 luStyle = m_pParser->GetTextStyle()->GetStyle();

	CFont 	newFont;
	LOGFONT	logFont;

	ChMemClearStruct( &logFont );

	lstrcpy( logFont.lfFaceName, 
					GetSettings()->GetFixedFontName() ); 

	logFont.lfHeight = (GetSettings()->GetPixelHt() * 
					(	GetSettings()->GetFixedFontSize())/ 72);


	logFont.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
	logFont.lfWeight = FW_LIGHT;

	newFont.CreateFontIndirect( &logFont );




	m_pParser->GetTextStyle()->SetFont( &newFont );
	m_pParser->GetTextStyle()->SetStyle( ChTxtWnd::textPreFormat | ChTxtWnd::textLeft );

	if ( lNewCount == - 1 )
	{									  
		lNewCount = lstrlen( pstrNewText );
	}

	chint32 lStart = 0;
	char    strLastChar = 0;

	while ( lStart < lNewCount )
	{
		char strChar = pstrNewText[lStart++];

		if (strChar == TEXT( '\n' ) && strLastChar != TEXT( '\r' )  )
		{
			m_pParser->AppendChar( TEXT( '\r' )  );
		}
		else 
		{ 
			m_pParser->AppendChar( strChar  );
		}
		strLastChar = strChar;
	} 

		m_pParser->CommitBuffer();

	// Restore the text style
	m_pParser->GetTextStyle()->SetStyle( luStyle );
	m_pParser->GetTextStyle()->SetFont( pOldFont );

}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlView::DisplayFile

------------------------------------------------------------------------------
	This method takes a text file and displays the file.
----------------------------------------------------------------------------*/

bool ChHtmlView::DisplayFile( const char* pstrFile,	const char* pstrMimeType /* = 0 */,
						chuint32 flOptions, const ChString& strURL )
{

	ChString strDocMime( pstrMimeType ? pstrMimeType : "" );

	int iMimeType;

	if ( strDocMime.IsEmpty() )
	{
		ChHTTPConn::GetMimeTypeByFileExtn( pstrFile, strDocMime );
	}

	iMimeType = ChHTTPConn::GetMimeType( strDocMime );

	bool boolSuccess = false;

	switch( iMimeType )
	{
		case ChHTTPConn::typeHTML:
		case ChHTTPConn::typeText:
		{
			ASSERT( false ); // should always be streamed
			//boolSuccess = DisplayHtmlFile( pstrFile, flOptions, strURL );
			break;
		}
		case ChHTTPConn::typeGIF:
		case ChHTTPConn::typeJPEG:
		case ChHTTPConn::typeBMP :
		{  // Handle files which contain only image

			ChDib *pDib = GetFrameMgr()->LoadDIB( strDocMime, pstrFile );
			if ( pDib )
			{
				boolSuccess = DisplayImage( pDib, flOptions, strURL );
			}
			break;
		}
		default :
		{
			boolSuccess = DisplayUnknown( pstrFile, flOptions, strURL, strDocMime );
			break;
		}
	}
	
	if ( boolSuccess && m_strDocTitle.IsEmpty() )
	{
		m_strDocTitle = strURL;
	}

	return boolSuccess;

}


/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlView::DisplayResource

------------------------------------------------------------------------------
	This method takes a resource block of type "HTML" and displays the
	data from that resource.  It's assumed that the resource is a zero-
	terminated text block.
----------------------------------------------------------------------------*/

void ChHtmlView::DisplayResource( int iResID, chuint32 flOptions,
									HINSTANCE hModule )
{
	#if defined( CH_MSW )

	HRSRC		hRes;
	HGLOBAL		hResBlock;

	#endif

	bool		boolSuccess = true;
	SetUpdateIndex( ChTxtWnd::GetTextCount() );


	#if defined( CH_MSW )
	{										// Open the resource
		if (0 == hModule)
		{
			hModule = AfxGetInstanceHandle();
		}

		hRes = ::FindResource( hModule, MAKEINTRESOURCE( iResID ),
								HTML_RES_TYPE );

		ASSERT( hRes );

		hResBlock = ::LoadResource( hModule, hRes );

		if (0 == hResBlock)
		{
			boolSuccess = false;
		}
	}
	#endif	// defined( CH_MSW )

	if (boolSuccess)
	{

		if (flOptions & ChHtmlWnd::fileReplace)
		{
			NewPage();
		}


		char*	pstrBuffer = 0;
		bool	boolDisplayAppend = CalcDisplayAppend();

		#if defined( CH_MSW )
		{
			pstrBuffer = (char*)LockResource( hResBlock );
		}
		#elif defined( CH_UNIX )
		{
			cerr << "XXX Get pstrBuffer & iBufferLen" << __FILE__ << ":" << __LINE__ << endl;

			pstrBuffer = "The code to get this text is not complete.";
		}
		#endif

		ASSERT( pstrBuffer );

		m_pParser->ParseText( pstrBuffer, strlen( pstrBuffer ) );

											// Did we add anything to the view ?

		if (ChTxtWnd::GetTextCount() != GetUpdateIndex() )
		{
											/* Commit all the changes made to
												the text view, this causes it
												to be rendered */

			ChTxtWnd::UpdateAppendText( GetUpdateIndex(), boolDisplayAppend );
		}
		// no prefetch allowed for resource files
		if (m_prefetchList.GetCount())
		{

			ChPosition pos = m_prefetchList.GetHeadPosition();

			while( pos != 0 )
			{
				ChPosition	prevPos = pos;
				ChHTMLPrefetch*	pFetch = m_prefetchList.GetNext( pos );

				m_prefetchList.Remove( prevPos );

				delete pFetch;

				pos = m_prefetchList.GetHeadPosition();
			}
		}

	}
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlView::DisplayImage

------------------------------------------------------------------------------
	This method takes a text file and displays the file.
----------------------------------------------------------------------------*/

bool ChHtmlView::DisplayImage( ChDib *pDib,	chuint32 flOptions,	const ChString& strURL )
{
	if ( flOptions & ChHtmlWnd::fileReplace )
	{
		NewPage();
	}


	SetDocURL( strURL );

    // Create our image wrapper
	ChInlineImageData *pImage = new ChInlineImageData;
	ASSERT( pImage );

	pImage->SetImage( pDib );
	// Insert it to our list
	GetImageList().Insert( strURL, pImage );

	ChSize		size( pDib->GetWidth(), pDib->GetHeight() );
	ChRect		spaceExtra( 0, 0, 0 , 0 );

	SetViewIndents( spaceExtra );
	
	// Add image object to Text wnd
	ChObjInline*		pObjImg = new ChObjInline( size, spaceExtra, 
									ChTxtWnd::objAttrMiddle, 0,
									m_pParser->GetTextStyle()->GetTextColor(), pImage );
	ChTxtObject		ImgObject( pObjImg );

	ASSERT( 0 != pObjImg );

	ImgObject.SetStyle( ChTxtWnd::textObject );

	// append the object to text wnd
	AppendObject( &ImgObject );

	// commit all the changes made to text view, this causes it to be render
	ChTxtWnd::UpdateAppendText( 0, CalcDisplayAppend() );


	return true;
}

/*----------------------------------------------------------------------------

	FUNCTION	||	ChHtmlView::DisplayUnknown

------------------------------------------------------------------------------
	This method takes a text file and displays the file.
----------------------------------------------------------------------------*/

bool ChHtmlView::DisplayUnknown( const char* pstrFile,
						chuint32 flOptions,
						const ChString& strURL,  const ChString& strMimeType )
{

	ChRect 		rtClient;
	GetClientRect( &rtClient );
	ChSize		size( rtClient.Width(), rtClient.Height() );
	ChRect		spaceExtra( 0, 0, 0 , 0 );
	bool		boolSuccess = true;


	if ( GetPlugInMgr()->HandleMimeType( strMimeType ) )
	{
		if ( flOptions & ChHtmlWnd::fileReplace )
		{
			NewPage();
		}

		ChInlinePluginData *pInline = new ChInlinePluginData( 0 );
		ASSERT( pInline );
		// Add image object to Text wnd
		ChObjInline*		pObjPlugIn = new ChObjInline( size, spaceExtra, 
										ChTxtWnd::objAttrMiddle, 0,
										0, pInline );	  

		ChTxtObject		ImgObject( pObjPlugIn );

		ASSERT( 0 != pObjPlugIn );

		ImgObject.SetStyle( ChTxtWnd::textObject );

		// append the object to text wnd
		AppendObject( &ImgObject );



		pObjPlugIn->SetMode( ChObjInline::embedFull );
		SetViewIndents( spaceExtra );


		if ( GetPlugInMgr()->LoadPlugInModule( this, pstrFile, strURL, strMimeType, pObjPlugIn ) )
		{
			SetDocURL( strURL );
			// commit all the changes made to text view, this causes it to be render
			ChTxtWnd::UpdateAppendText( 0, CalcDisplayAppend() );
		}
		else
		{
			boolSuccess = false;
		}
	}
	return boolSuccess;
}

chint32 ChHtmlView::RedirectStream( const char* pstrBuffer, chint32 lStart, chint32 lCount )
{
	if ( GetFrameMgr()->IsRedirectStream( GetFrameName() ) )
	{
		SetRedirectInfo( true, lStart );
		return lCount;
	}
	return lStart;
}






bool ChHtmlView::CalcDisplayAppend()
{
	bool	boolDisplayAppend = false;

	if (AlwaysDisplayAppend())
	{
		ChPoint		ptPos = GetDeviceScrollPosition();
		bool		boolBottomOfView;
											/* If we are currently at the
												bottom of the view or if the
												view has no scroll bars we will
												display what we append */

		boolBottomOfView = (ptPos.y + ChTxtWnd::GetViewHeight()) <=
								GetCanvasHeight();

		if (boolBottomOfView || (GetCanvasHeight() <= GetViewHeight()))
		{
			boolDisplayAppend = true;
		}
	}

	return boolDisplayAppend;
}


/*----------------------------------------------------------------------------
	This method will return true if the link is in the prefetch list
----------------------------------------------------------------------------*/
bool ChHtmlView::PrefetchedLink( ChString &strLink )
{
	// if this append  had any prefetch tags, call the virtual function
	if (m_prefetchList.GetCount())
	{

		ChPosition pos = m_prefetchList.GetHeadPosition();

		while( pos != 0 )
		{
			ChHTMLPrefetch*	pFetch = m_prefetchList.GetNext( pos );
			
			const char *pstrDoc = strrchr( pFetch->GetHREF(), TEXT( '/' ) );
			if ( !pstrDoc )
			{
				pstrDoc = pFetch->GetHREF();
			}
			else
			{  	// point to the document name
				pstrDoc++;
			}
			
			ChString strDoc;

			int iDoc = strLink.ReverseFind( TEXT( '/' ) );
			if ( iDoc == -1 )
			{
				strDoc = strLink;	
			}
			else
			{
				strDoc = strLink.Right( strLink.GetLength() - iDoc - 1 );
			}

			if ( strDoc == pstrDoc )
			{
				return true;
			}
		}
	}

	return false;
}

void ChHtmlView::LoadBkPattern( const ChString& strURL )
{
	ChHtmlBkPatternReq* pInfo = new ChHtmlBkPatternReq( this );

	GetFrameMgr()->LoadURL( strURL, pInfo );
}


void ChHtmlView::LoadInlineImage( const ChString& strURL, ChObjInline *pInLine )
{
	ChString strFile, strMimeType;
	if ( GetFrameMgr()->GetHTTPConn()->
			GetCachedURL( strURL, strFile, strMimeType, 0, 
					ChHTTPConn::returnVisited ) )
	{

		ChDib* pDib = GetFrameMgr()->LoadDIB( strMimeType, strFile );

		if ( pDib )
		{
			if ( pInLine->GetImageData() )
			{
				pInLine->GetImageData()->SetImage( pDib );
			}
			else
			{
				ChInlineImageData* pData = new ChInlineImageData;
				ASSERT( pData );
				GetImageList().Insert( strURL, pData );
				pData->SetImage( pDib );
				// remove the plugin object if any
				pInLine->ShutdownPlugin();
				// Set the image object
				pInLine->SetImageData( pData );
			}
		}
		else
		{ 
			if ( GetPlugInMgr()->HandleMimeType( strMimeType ) )
			{	// Load the plugin
				GetPlugInMgr()->LoadPlugInModule( this, 
							strFile, strURL, strMimeType, pInLine );
			}
			else
			{ // display broken image
			}
		}
	}
	else
	{
		ChHtmlInlineReq* pInfo = new ChHtmlInlineReq( this, pInLine );

		GetFrameMgr()->LoadURL( strURL, pInfo );
	}
}

void ChHtmlView::SpawnPlugInRequest( const ChString& strURL, ChPlugInStream* pStream,
										const char* pstrWindow )
{ 	
	if ( pStream )
	{  // Request a stream for Plugin
		ChHtmlPluginReq * pReq = new ChHtmlPluginReq( this, pStream );
		ASSERT( pReq );

		GetFrameMgr()->LoadURL( strURL, pReq );
	}
	else
	{	// Load a new URL into the current window
		ChString strWindow( pstrWindow );

		if ( strWindow == CURRENT_WINDOW )
		{
			strWindow = GetFrameName();	
		}
		
		GetFrameMgr()->LoadURL( strURL, strWindow );
	}
}

bool ChHtmlView::NotifyInline( const char *pstrBuf )
{
	return m_pFrameMgr->OnViewInline( this, pstrBuf  );
}

void ChHtmlView::NotifySubmitForm( const ChString& strAction, 
						const ChString& strMD5, const ChString& strData )
{
	m_pFrameMgr->OnViewSubmitForm( this, strAction, strMD5, strData  );
}

void ChHtmlView::CreatePane( ChHtmlPane* pPane )
{
	m_pFrameMgr->CreatePane( this, pPane );
}


/*----------------------------------------------------------------------------
	This function will return the current indent in pixels
----------------------------------------------------------------------------*/

int ChHtmlView::GetCurrentIndent()
{
	return m_pParser->GetCurrentIndent();
}


ChPlugInMgr* ChHtmlView::GetPlugInMgr(  )
{
	if ( !m_pluginMgr )
	{
		m_pluginMgr = new ChPlugInMgr();
		ASSERT( m_pluginMgr );	
	}

	return m_pluginMgr;
}

ChHtmlSettings*	ChHtmlView::GetSettings()
{
	return m_pFrameMgr->GetSettings();	
}

void ChHtmlView::UnloadPlugins()
{
	for ( int i = GetObjectCount() - 1; i >= 0; i-- )
	{
		ChTextObject* pObject = GetObjectTable()[i];
		if ( ChTextObject::objectPlugin == pObject->GetType() )
		{
			((ChObjInline*)pObject)->ShutdownPlugin();
		}
	}
}

void ChHtmlView::UnloadImages()
{
	for ( int i = GetObjectCount() - 1; i >= 0; i-- )
	{
		ChTextObject* pObject = GetObjectTable()[i];
		if ( ChTextObject::objectImage == pObject->GetType() )
		{
			((ChObjInline*)pObject)->SetImageData( 0 );
		}
	}

											// Delete all the image 
	{ 
		ZapInlineData		zapImage;

	  	GetImageList().Infix( zapImage );

		GetImageList().Erase();
	}

	if ( m_idTimer )
	{
		KillTimer( m_idTimer );		 
		m_idTimer = 0;
	}

	// Remove all the images
	InvalidateRect( 0 );
}

// UE
void ChHtmlView::ChangeCursor(tagCursors newCursor) {
	m_hCursor = m_hHTMLCursors[ newCursor ];
	::SetCursor( m_hHTMLCursors[ newCursor ] );
}


// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
