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

	This file contains the implementation of the ChHTMLStreamManager class notification
	methods.
----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"
#include <ChConst.h>
#include <ChHtpCon.h>
#include <ChHtmWnd.h>  
#include <ChDibImage.h>
#include <ChDIBDecoder.h>
#include <ChGIFDecoder.h>
#include <ChJPEGDecoder.h>
#include <ChMngImage.h>
#include <ChPane.h>

#include "ChHtmlView.h"
#include "ChTxtObj.h"
#include "ChPlgInMgr.h"
#include "ChHTMLStream.h"
#include "ChHtmlPane.h"

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	ChHtmlReqInfo class	member functions
----------------------------------------------------------------------------*/

ChHtmlReqInfo::ChHtmlReqInfo(  ChHtmlView* pView ) :
								m_uReqID( 0 ),
								m_iError( 0 ),
								m_pFrameMgr( 0 )
{
	m_strViewName = pView->GetFrameName();
	m_uViewID  = pView->GetPageNumber();
}

void ChHtmlReqInfo::SetFrameMgrInfo( ChHtmlWnd* pWnd ) 	
{  
	m_pFrameMgr = pWnd; 
	m_uReqID    = pWnd->GetPageID();
}

/*----------------------------------------------------------------------------
	ChHtmlInlineReq class	member functions
----------------------------------------------------------------------------*/

ChHtmlInlineReq::~ChHtmlInlineReq() 
{
	delete m_pDib;
}

/*----------------------------------------------------------------------------
	ChHtmlBkPatternReq class	member functions
----------------------------------------------------------------------------*/

ChHtmlBkPatternReq::~ChHtmlBkPatternReq() 
{
	delete m_pDib;
}


/*----------------------------------------------------------------------------
	ChHTMLStreamManager class	member functions
----------------------------------------------------------------------------*/


int ChHTMLStreamManager::NewStream( chparam requestData, pChHTTPStream pStream, bool boolSeekable  )
{
	ChHtmlReqInfo* 	pLoadInfo = (ChHtmlReqInfo*)requestData;

	if ( pLoadInfo  )
	{ 
		pLoadInfo->SetURL( pStream->GetURL() );
		pLoadInfo->SetMimeType( pStream->GetMimeType()  );
			
	}

	return streamAsFile;
}

chint32 ChHTMLStreamManager::WriteReady( chparam requestData, pChHTTPStream pStream, chint32 iBytes )
{
	return iBytes;
}

chint32 ChHTMLStreamManager::Write( chparam requestData, pChHTTPStream pStream, 
						chint32 lOffset, chint32 lLen, const char* pBuffer )
{
	return lLen;
}


void ChHTMLStreamManager::StreamAsFile(chparam requestData, pChHTTPStream pStream, const char* fname)
{

	ChHtmlReqInfo* 	pLoadInfo = (ChHtmlReqInfo*)requestData;

	if ( pLoadInfo == 0 )
	{ // What is this ?
		return;	
	}

	// Set the cache file name
	pLoadInfo->SetFile( pStream->GetCacheFilename()  );

#if defined ( CH_PUEBLO_PLUGIN )
	// The mime type can change due to a bug in IE and Netscape 2.0
	pLoadInfo->SetMimeType( pStream->GetMimeType()  );
#endif

	// If it is a image decode it now

	if ( pLoadInfo->GetFrameMgr()->GetPageID() == pLoadInfo->GetReqID() ) 
	{ // Request has been canceled
		switch( pLoadInfo->GetType() )
		{
			case ChHtmlReqInfo::loadBkPattern :
			{
				ChHtmlBkPatternReq*		pBkInfo = (ChHtmlBkPatternReq*)pLoadInfo;

				pBkInfo->SetImage( pLoadInfo->GetFrameMgr()->
								LoadDIB( pStream->GetMimeType(), fname ) );
				break;
			}
			case ChHtmlReqInfo::loadInline :	  
			{
				ChHtmlInlineReq*		pInlineInfo = (ChHtmlInlineReq*)pLoadInfo;

				pInlineInfo->SetImage( pLoadInfo->GetFrameMgr()->
								LoadDIB( pStream->GetMimeType(), fname ) );
				break;
			}
			default :
			{
				break;
			}
		}
	}

}


void ChHTMLStreamManager::DestroyStream( chparam requestData, pChHTTPStream pStream, int iReason )
{
 	ChHtmlReqInfo* pLoadInfo = (ChHtmlReqInfo*)requestData;

	if ( pLoadInfo == 0 )
	{ // No idea what this is
		return;
	}

	int iType  	=   pLoadInfo->GetType();
	if ( iType != ChHtmlReqInfo::loadURL && 
				pLoadInfo->GetFrameMgr()->GetPageID() != pLoadInfo->GetReqID() ) 
	{ // Request has been canceled
	  	delete pLoadInfo;
	  	return; 
	}

	if ( iReason )
	{

		pLoadInfo->SetError( pStream->GetErrorMsg(), iReason );


		if( !pLoadInfo->GetFrameMgr()->PostMessage( WM_CHACO_HTTP_MSG, 
								ChHtmlReqInfo::loadError, (LPARAM)pLoadInfo ) )
		{
			TRACE1("Post message failed %ld\n", ::GetLastError() );
		}
	}
	else
	{

		if( !pLoadInfo->GetFrameMgr()->PostMessage( WM_CHACO_HTTP_MSG, 
							iType, (LPARAM)pLoadInfo ) )
		{
			TRACE1("Post message failed %ld\n", ::GetLastError() );
		}
	}
}


/*----------------------------------------------------------------------------
	ChHtmlWnd class	member functions
----------------------------------------------------------------------------*/

ChHTTPConn* ChHtmlWnd::GetHTTPConn()	 
{ 
#if !defined ( CH_PUEBLO_PLUGIN )
	if ( m_pHttpConnection == 0 )
	{	
		m_phtmlStreamMgr = new  ChHTMLStreamManager( this );
		ASSERT( m_phtmlStreamMgr );

		m_pHttpConnection = new ChHTTPSocketConn( m_phtmlStreamMgr );
		ASSERT( m_pHttpConnection );
		
		m_boolMyConn = true;	
	}
#endif
	return m_pHttpConnection; 
}

bool ChHtmlWnd::VisitedURL( const ChString &strLink, const char * pstrDefault /* = 0 */ )
{
	
	if ( GetHTTPConn()->IsVisitedURL( strLink, pstrDefault ) )
	{
		return true;
	}
	else
	{
		return false;
	}
}

void ChHtmlWnd::AbortFormatting()					
{
	m_boolAbortFormatting = true;
	GetHTTPConn()->AbortRequests( false, m_phtmlStreamMgr );
}


void ChHtmlWnd::LoadURL( const ChString& strURL, const char* pstrWindowName /* = 0 */ )
{
	ChString strTarget( pstrWindowName ? pstrWindowName : TOP_WINDOW );

	ChHtmlView *pView = GetHtmlViewByName( strTarget );

	if ( !pView )
	{
		pView = GetDefaultView();
		strTarget = pView->GetFrameName();
	}

	// Abort all previous out standing requests
	GetHTTPConn()->AbortRequests( false, m_phtmlStreamMgr );

	if ( pView ==  GetDefaultView() )
	{

		while ( m_pRedirectStack )
		{
			while( m_pRedirectStack->GetCount() )
			{
				delete  m_pRedirectStack->RemoveHead();
			}
			delete m_pRedirectStack;
			m_pRedirectStack = 0;		
		}

		// Delete all frames
		while ( m_htmlViewList.GetCount() > 1 )
		{

			ChHtmlViewObj*	pFrameObj = m_htmlViewList.RemoveTail();
			delete pFrameObj;
		}

	}


	ChHtmlReqInfo* pInfo = new ChHtmlURLReq( pView, strTarget ); 

	pInfo->SetFrameMgrInfo( this );



	GetHTTPConn()->GetURL( strURL, (chparam)pInfo, 0, 0, m_phtmlStreamMgr );
}


void ChHtmlWnd::PostURL( const ChString& strURL, const ChString& strData, 
									const char* pstrWindowName /* = 0 */ )
{
	ChString strTarget( pstrWindowName ? pstrWindowName : TOP_WINDOW );

	ChHtmlView *pView = GetHtmlViewByName( strTarget );

	if ( !pView )
	{
		pView = GetDefaultView();
	}
	
	GetHTTPConn()->AbortRequests( false, m_phtmlStreamMgr );

	if ( pView ==  GetDefaultView() )
	{
		while ( m_pRedirectStack )
		{
			while( m_pRedirectStack->GetCount() )
			{
				delete  m_pRedirectStack->RemoveHead();
			}
			delete m_pRedirectStack;
			m_pRedirectStack = 0;		
		}

		//NewPage();


		// Delete all frames
		while ( m_htmlViewList.GetCount() > 1 )
		{

			ChHtmlViewObj*	pFrameObj = m_htmlViewList.RemoveTail();
			delete pFrameObj;
		}

	}

	ChHtmlReqInfo* pInfo = new ChHtmlURLReq( pView, strTarget ); 

	pInfo->SetFrameMgrInfo( this );


	GetHTTPConn()->PostURL(  strURL, strData, 
							strData.GetLength(),
							(chparam)pInfo, 0, 0, m_phtmlStreamMgr  );
}


void ChHtmlWnd::PrefetchURL( const ChString& strURL, const char * pstrDef /* = 0 */ )
{
	if ( !VisitedURL( strURL, pstrDef ) )
	{
		// prefetch only if we don't have this document
		GetHTTPConn()->GetURL( strURL, 0, pstrDef, 
					ChHTTPConn::PrefetchURL, m_phtmlStreamMgr );
	}
}

void ChHtmlWnd::LoadURL( const ChString& strURL, ChHtmlReqInfo* pReqInfo )
{
	ChHtmlView *pView = GetHtmlViewByName( pReqInfo->GetViewName() );
	ASSERT( pView );

	pReqInfo->SetFrameMgrInfo( this );
	GetHTTPConn()->GetURL( strURL, (chparam)pReqInfo, 
					pView->GetDocBaseURL(), 0, m_phtmlStreamMgr );
	//TRACE3("HTMLWnd: request %d: GET %s | %s\n", pReqInfo->GetReqID(),
	//				(LPCSTR)pReqInfo->GetURL(), strURL);
}

void ChHtmlWnd::PostURL( const ChString& strURL, const ChString& strData, ChHtmlReqInfo* pReqInfo )
{
	ChHtmlView *pView = GetHtmlViewByName( pReqInfo->GetViewName() );
	ASSERT( pView );

	pReqInfo->SetFrameMgrInfo( this );

	GetHTTPConn()->PostURL( strURL, strData, 
							strData.GetLength(),
							(chparam)pReqInfo, pView->GetDocBaseURL(), 
							0, m_phtmlStreamMgr );
}



ChDib* ChHtmlWnd::LoadDIB( const ChString& strMimeType, const char* pstrFilename )
{

	int iMimeType = ChHTTPConn::GetMimeType( strMimeType );

	ChDib*	pDib = 0;

	switch ( iMimeType )
	{
		case ChHTTPConn::typeGIF:
		{								/* Update the image */

			pDib = new ChDib;
			ASSERT( pDib );
			// create the gif object
			ChGifDecoder gifDecoder( pDib );
			if ( gifDecoder.Load( pstrFilename ) )
			{
				return( pDib );
			}
			else
			{
				return pDib;
			}
			//break;
		}
		case ChHTTPConn::typeJPEG:
		{								/* Update the image */
			pDib = new ChDib;
			ASSERT( pDib );
			// create the JPEG object
			ChJPEG jpegDecoder( pDib );
			if ( jpegDecoder.Load( pstrFilename, ChImageDecoder::load8Bit ) )
			{
				return( pDib );
			}
			else
			{
				return pDib;
			}
			//break;
		}
		case ChHTTPConn::typeBMP :
		{								/* Update the image */
			// create the Dib object
			ChDib *pDib = new ChDib;
			ChDibDecoder dibDecoder( pDib );
			if ( dibDecoder.Load( pstrFilename ) )
			{
				return( pDib );
			}
			else
			{
				return pDib;
			}
			//break;
		}
		case ChHTTPConn::typeNG :	// PNG, MNG/JNG
		{
			ChMngImage *pImage = new ChMngImage();
			pImage->Load( pstrFilename );
			return pImage;
		}
	}

	return 0;
}

/*----------------------------------------------------------------------------
	This method called when HTTP posts a message 
----------------------------------------------------------------------------*/

LONG ChHtmlWnd::OnHTTPNotificaton( UINT wParam, LONG lParam )
{
	ChHtmlReqInfo* pInfo = (ChHtmlReqInfo*)lParam;

	if ( GetPageID() != pInfo->GetReqID() ) 
	{ // Request has been canceled
			//TRACE2("HTMLWnd: request %d completed, but wanted %d.\n",
			//				pInfo->GetReqID(), GetPageID());
	  	delete pInfo;
	  	return 0; 
	}

	//TRACE1("HTMLWnd: request %d: ", pInfo->GetReqID());

	switch( wParam )
	{ 
		case ChHtmlReqInfo::loadURL :
		{

			ChHtmlURLReq*		pLoadInfo = (ChHtmlURLReq*)pInfo;
			ChHtmlView *pView = GetHtmlViewByName( pLoadInfo->GetTarget() );

			//TRACE0("LoadURL: ");
			if ( pView == GetDefaultView() )
			{
				//TRACE0("LoadComplete\n");
				OnLoadComplete( pInfo->GetFile(), pInfo->GetURL(), pInfo->GetMimeType(),
										 					(chparam)pInfo ); 
			}
			else if ( pView )
			{
				//TRACE0("StreamFile\n");
			 	StreamFile( pInfo->GetFile(),	pInfo->GetMimeType(), 
			 								ChHtmlWnd::fileReplace, 
			 								pInfo->GetURL(), pView );
			}
			break;
		}
		case  ChHtmlReqInfo::loadPlugInReq :
		{
			//TRACE0("LoadPlugin\n");
			// did we get a new page after this load
			ChHtmlPluginReq*		pLoadInfo = (ChHtmlPluginReq*)pInfo;
			ChHtmlView *pView = GetHtmlViewByName( pLoadInfo->GetViewName() );

			if ( pView && pView->GetPageNumber()  == pLoadInfo->GetViewID() )
			{
				ChPlugInStream* pStream = pLoadInfo->GetPlugInStream();
				pStream->OnLoadComplete( pInfo->GetFile(), pInfo->GetURL(), pInfo->GetMimeType() );
			}
			break;
		}
		case  ChHtmlReqInfo::loadInline :
		{
			//TRACE0("LoadInline\n");
			ChHtmlInlineReq*		pLoadInfo = (ChHtmlInlineReq*)pInfo;

			ChObjInline* pImage = pLoadInfo->GetInlineData();

			ChHtmlView *pView = GetHtmlViewByName( pLoadInfo->GetViewName() );

			if ( pView && pView->GetPageNumber()  == pLoadInfo->GetViewID() )
			{
				ChDib *pDib = pLoadInfo->GetImage();
				pLoadInfo->SetImage( 0 ); // we want the Dib to stay around
				if ( pDib )
				{
					ChSize sizeOld, sizeNew( 0, 0) ;
					// old size
					pImage->GetImageSize( sizeOld );
					// set the image into our object wrapper
					if ( 0 == pImage->GetImageData() )
					{
						ChInlineImageData* pData = new ChInlineImageData;
						ASSERT( pData );
						pView->GetImageList().Insert( pInfo->GetURL(), pData );
						// remove the plugin object if any
						pImage->ShutdownPlugin();
						pImage->SetImageData( pData );
					}
					// Set the image object
					pImage->GetImageData()->SetImage( pDib );
					pImage->GetImageSize( sizeNew );
					pView->UpdateObject( pImage, true ); //(sizeOld != sizeNew) != FALSE );

					if (m_pPaneMgr && (sizeOld != sizeNew))
					{
						ChPane*		pPane; 

						pPane = m_pPaneMgr->FindPane( pView->GetFrameName() );
						if (pPane && pPane->IsSizeToFit())
						{
											/* Inline image has changed the
												document size */

							m_pPaneMgr->RecalcLayout( pPane );
						}
					}
				}
				else
				{ 
					 
					ChSize sizeObj;
					// old size
					pImage->GetObjectSize( sizeObj );  
					
					if ( sizeObj.cx == 8 && sizeObj.cy == 8 )
					{
						sizeObj.cx = (80 * pView->GetViewWidth())/100;	
						sizeObj.cy = (40 * pView->GetViewHeight())/100;
						
						pImage->SetObjectSize( sizeObj );	
					}

					if (pView->GetPlugInMgr()->HandleMimeType( pInfo->GetMimeType() ))
					{
											// Load the plugin
						pView->GetPlugInMgr()->
								LoadPlugInModule( pView, pInfo->GetFile(),
													pInfo->GetURL(),
													pInfo->GetMimeType(),
													pImage );
						InvalidateRect( NULL, true );

						if (m_pPaneMgr)
						{
							ChPane*		pPane; 

							pPane = m_pPaneMgr->FindPane( pView->GetFrameName() );
							if (pPane && pPane->IsSizeToFit())
							{
											/* Inline image has changed the
												document size */

								m_pPaneMgr->RecalcLayout( pPane );
							}
						}
					}
					else
					{						// Display broken image
						pImage->CreateBrokenImagePlaceholder();
						pView->UpdateObject( pImage );
					}

				}
			}
			break;
		}
		case  ChHtmlReqInfo::loadBkPattern :
		{
			//TRACE0("LoadBkPattern\n");
			ChHtmlBkPatternReq*		pLoadInfo = (ChHtmlBkPatternReq*)pInfo;
			//SetRedraw();  // turn it on 
			if ( pLoadInfo->GetImage() )
			{
				ChHtmlView *pView = GetHtmlViewByName( pLoadInfo->GetViewName() );
				if ( pView )
				{
					pView->SetBackPattern( pLoadInfo->GetImage() );
					// We need to save this else the object will be deleted by
					// ChHtmlBkPatternReq
					pLoadInfo->SetImage( 0 );
				}
			}
			break;
		}

		case ChHtmlReqInfo::loadFrameReq:
		{
			//TRACE0("LoadFrame\n");
			ChHtmlFrameReq*	pLoadInfo = (ChHtmlFrameReq*)pInfo;
			ChHtmlView*		pView;

			pView = GetHtmlViewByName( pLoadInfo->GetViewName() );

			if (pView)
			{
				chflag32	flDispOptions;

				flDispOptions =
					pLoadInfo->GetOptions() & ChHtmlPane::optionFileAppend ?
									ChHtmlWnd::fileAppend :
									ChHtmlWnd::fileReplace;

				if (DisplayFile( pInfo->GetFile(), pInfo->GetMimeType(),
									flDispOptions, pInfo->GetURL(),
									pView->GetFrameName() ))
				{
					ChPane*		pPane; 

					if (m_pPaneMgr &&
						(pPane = m_pPaneMgr->FindPane( pView->GetFrameName() )))
					{
											// Finally show the pane
						pPane->Show();
					}
					else
					{						// our frame do the show

						ChPosition pos = m_htmlViewList.GetHeadPosition();

						while (pos)
						{
							ChHtmlViewObj*	pFrameObj = m_htmlViewList.GetNext( pos );

							if (0 == pFrameObj->GetHtmlView()->GetFrameName().CompareNoCase( pView->GetFrameName() ))
							{
								if ( pFrameObj->GetFrameWnd() )
								{
									pFrameObj->GetFrameWnd()->ShowWindow( SW_SHOWNORMAL );
								}
								break;
							}
							else if (0 == pFrameObj->GetAltFrameName().CompareNoCase( pView->GetFrameName() ))
							{
								if ( pFrameObj->GetFrameWnd() )
								{
									pFrameObj->GetFrameWnd()->ShowWindow( SW_SHOWNORMAL );
								}
								break;
							}
						}
					}
		
				}
			}
			break;
		}
		case ChHtmlReqInfo::loadError :
		{
			//TRACE0("LoadError\n");

			ChString strMsg( pInfo->GetURL() );
			strMsg += TEXT( " : " );
			strMsg += pInfo->GetErrorMsg();

			OnTrace( strMsg, traceError );
			
			switch( pInfo->GetType() )
			{ 
				case ChHtmlReqInfo::loadURL :
				{	// call the virtual function to notify
					OnLoadError( pInfo->GetError(), 
							pInfo->GetErrorMsg(), pInfo->GetURL(), 0 );
					break;
				}
				case  ChHtmlReqInfo::loadPlugInReq :
				{
					ChHtmlPluginReq*		pLoadInfo = (ChHtmlPluginReq*)pInfo;
					ChPlugInStream* pStream = pLoadInfo->GetPlugInStream();
					pStream->OnLoadError( pInfo->GetError(), pInfo->GetURL() );
					break;
				}
				case ChHtmlReqInfo::loadInline :
				{  // display broken image
					ChHtmlInlineReq*		pLoadInfo = (ChHtmlInlineReq*)pInfo;
					ChObjInline* pImage = pLoadInfo->GetInlineData();
					ChHtmlView *pView = GetHtmlViewByName( pLoadInfo->GetViewName() );

					if ( pView && pView->GetPageNumber()  == pLoadInfo->GetViewID() )
					{
						pImage->CreateBrokenImagePlaceholder();
						pView->UpdateObject( pImage );
					}
					break;
				}
				case ChHtmlReqInfo::loadBkPattern :
				{
					SetRedraw();  // turn it on 
					InvalidateRect( NULL );
					break;
				}
			}
			break;
		}
		default :
		{
			break;
		}
	}
	// Delete the user data we associated with this request
	delete 	pInfo;
//#ifdef _DEBUG
//	afxDump.Flush();
//#endif

	return 0;
}

// $Log$
// Revision 1.2  2003/07/04 11:26:41  uecasm
// Update to 2.60 (see help file for details)
//
// Revision 1.1.1.1  2003/02/03 18:54:13  uecasm
// Import of source tree as at version 2.53 release.
//
