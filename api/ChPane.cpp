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

	This file contains the implementation of the ChPaneManager class.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#if !defined(CH_PUEBLO_PLUGIN)
#include "resource.h"
#else
#include "vwrres.h"
#endif

#include <ChTypes.h>
#include <ChConst.h>
#include <ChCore.h>
#include <ChReg.h>
#include <ChPane.h>
#include <ChSplit.h>
//#include <ChDib.h>
#include <ChHtmWnd.h>


#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define CLASS_PANE_FRAME			"Pueblo ChPane Frame (no. %lu)"

#define SPLIT_PANE_TEXT				0
#define SPLIT_PANE_GRAF				1

#define ATTR_PANE					"xch_pane"
#define ATTR_NAME					"name"
#define ATTR_PANE_TITLE				"xch_pane_title"
#define ATTR_PANE_OPTIONS			"xch_pane_options"

#define ATTR_HEIGHT					"height"
#define ATTR_WIDTH					"width"
#define ATTR_MIN_HEIGHT				"xch_min_height"
#define ATTR_MIN_WIDTH				"xch_min_width"
#define ATTR_VSPACE					"vspace"
#define ATTR_HSPACE					"hspace"

#define PANE_OPTIONS_DELIM			" ,;\t"

#define RGBBLACK					RGB(0,0,0)
#define RGBGRAY						RGB(128,128,128)
#define RGBLTGRAY					RGB(192,192,192)
#define RGBWHITE					RGB(255,255,255)


/*----------------------------------------------------------------------------
	Forward function definitions
----------------------------------------------------------------------------*/

#if defined( CH_MSW )

LRESULT CALLBACK PaneFrameCallback( HWND hWnd, UINT uMsg,
									WPARAM wParam, LPARAM lParam );

#endif	// defined( CH_MSW )


/*----------------------------------------------------------------------------
	Type definitions
----------------------------------------------------------------------------*/

											/* The following structure
												associates a pane option
												name with a flag bit */
typedef struct
{
	char*		pstrName;
	chflag32	flOption;

} PaneOptionsNameType;


/*----------------------------------------------------------------------------
	Global variables
----------------------------------------------------------------------------*/

const PaneOptionsNameType	paneOptionsList[] =
									{	{ "normal", paneOverlapped },	// Backwards
										{ "overlapped", paneOverlapped },
										{ "floating", paneFloat },
										{ "internal", paneInternal },
										{ "banner", paneInternal },		// Backwards
										{ "sizeable", paneSizeable },
										{ "closeable", paneCloseable },
										{ "small_title", paneSmallTitle },
										{ "noscroll", paneNoScroll },
										{ "top", paneAlignTop },
										{ "bottom", paneAlignBottom },
										{ "fit", paneSizeToFit },
										{ "persistent", panePersistent },
										{ "force", paneForce },
										{ "_frame", paneFrame }
									};


/*----------------------------------------------------------------------------
	ChPaneMgr class
----------------------------------------------------------------------------*/

// UE: changed these from "ChString" to "char *", initially because I thought
//     they were causing a problem.  Later determined that they aren't, but
//     left them changed because they really don't need the overhead of
//     being "ChString"s, and it doesn't break anything to leave as is.
const char *ChPaneMgr::strTextName = "text";
const char *ChPaneMgr::strGraphicName = "graphic";

ChPaneMgr::ChPaneMgr( ChCore* pCore ) : m_pCore( pCore )
{
	ChPane*		pPane;

	pPane = new ChPane( ChPane::paneText, strTextName, this );
	ASSERT( pPane );
 	m_list.AddTail( pPane );

	pPane = new ChPane( ChPane::paneGraphic, strGraphicName, this );
	ASSERT( pPane );
	m_list.AddTail( pPane );
}


ChPaneMgr::~ChPaneMgr()
{
	m_list.Empty();
}


ChPane* ChPaneMgr::FindPane( const ChString& strName )
{
	ChPane*		pPane = 0;
	ChPosition	pos = m_list.GetHeadPosition();

	while ((0 != pos) && (0 == pPane))
	{
		ChPane*		pCurrPane = m_list.Get( pos );

		if (strName == pCurrPane->GetName())
		{
			pPane = pCurrPane;
		}
		else
		{
			m_list.GetNext( pos );
		}
	}

	return pPane;
}


ChPane* ChPaneMgr::CreatePane( const ChString& strName, chparam userData,
								chint16 sIdealWidth, chint16 sIdealHeight,
								chflag32 flOptions )
{
	ChPane*		pPane = FindPane( strName );

	if (0 == pPane)
	{
		pPane = new ChPane( strName, this, sIdealWidth, sIdealHeight,
								userData, flOptions );
		ASSERT( pPane );

		m_list.AddTail( pPane );
	}

	return pPane;
}


void ChPaneMgr::DestroyPane( const ChString& strName )
{
	ChPosition	pos = m_list.GetHeadPosition();
	bool		boolFoundAndDestroyed = false;

	while (pos && !boolFoundAndDestroyed)
	{
		if (strName == m_list.Get( pos )->GetName())
		{
			m_list.Remove( pos );
			boolFoundAndDestroyed = true;
		}
		else
		{
			m_list.GetNext( pos );
		}
	}
}


void ChPaneMgr::ShowAllPanes( const ChModuleID& idModule, bool boolShow )
{
	ChPosition	pos = m_list.GetHeadPosition();

	while (pos)
	{
		if (idModule == m_list.Get( pos )->GetModuleID())
		{
			m_list.Get( pos )->Show( boolShow );
		}

		m_list.GetNext( pos );
	}
}


void ChPaneMgr::DestroyAllPanes( const ChModuleID& idModule,
									bool boolDestroyFramePanes )
{
	ChPosition	pos = m_list.GetHeadPosition();

	while (pos)
	{
		ChPane*		pPane = m_list.Get( pos );
		bool		boolDestroyed = false;

		if (idModule == pPane->GetModuleID())
		{
			if (boolDestroyFramePanes || (!(pPane->GetOptions() & paneFrame)))
			{
											// Hide and destroy the pane
				pPane->Show( false );

				if ((ChPane::paneText == pPane->GetType()) ||
					(ChPane::paneGraphic == pPane->GetType()))
				{
					pPane->SetOwner( 0, 0, 0 );
				}
				else
				{								// Remove the item
					m_list.Remove( pos );
				}

				boolDestroyed = true;
			}
		}

		if (boolDestroyed)
		{
											// Start at the top again
			pos = m_list.GetHeadPosition();
		}
		else
		{
			m_list.GetNext( pos );
		}
	}
}


void ChPaneMgr::RecalcLayout( ChPane* pChangingPane )
{
	if (pChangingPane)
	{
		pChangingPane->SizeToFit();
	}
}


/*----------------------------------------------------------------------------
	ChPaneMgr protected methods
----------------------------------------------------------------------------*/

chuint32	ChPaneMgr::m_luClassNumber = 0;

ChString ChPaneMgr::FindClassName( bool boolCloseable, chuint16 suIconID )
{
	ChPosition	pos = m_wndClassList.GetHeadPosition();
	bool		boolFound = false;
	ChString		strName;
											/* We need to create new classes to
												manage icons and whether a
												window is closeable.  This
												list keeps track of created
												classes. */
	while (pos && !boolFound)
	{
		ChPaneWindowClass*		pWindowClass = m_wndClassList.Get( pos );

		if ((boolCloseable == pWindowClass->IsCloseable()) &&
			(suIconID == pWindowClass->GetIconID()))
		{
			boolFound = true;
			strName = pWindowClass->GetName();
		}

		m_list.GetNext( pos );
	}

	if (!boolFound)
	{
		ChPaneWindowClass*		pWindowClass;

											// Create a new class

		strName = RegisterPaneClass( boolCloseable, suIconID );
		if (!strName.IsEmpty())
		{									// Add the class to the list

			pWindowClass = new ChPaneWindowClass( strName, boolCloseable,
													suIconID );
			m_wndClassList.AddTail( pWindowClass );
		}
	}

	return strName;
}


ChString ChPaneMgr::RegisterPaneClass( bool boolCloseable, chuint16 suIconID )
{
	#if defined( CH_MSW )
	{
		WNDCLASS	classStruct;
		bool		boolClassCreated;
		char		buffer[80];
											// Register the class
		ChMemClearStruct( &classStruct );

		classStruct.style = CS_GLOBALCLASS | CS_DBLCLKS | CS_OWNDC;
		if (!boolCloseable)
		{
			classStruct.style |= CS_NOCLOSE;
		}

		classStruct.lpfnWndProc = PaneFrameCallback;
		#if defined( CH_PUEBLO_PLUGIN )
		classStruct.hInstance = AfxGetInstanceHandle();
		#else
		classStruct.hInstance = PuebloDLL.hModule;
		#endif
		classStruct.hCursor = LoadCursor( 0, IDC_ARROW );
		#if defined( CH_PUEBLO_PLUGIN )
		classStruct.hIcon = LoadIcon( AfxGetInstanceHandle(),
										MAKEINTRESOURCE( suIconID ) );
		#else
		classStruct.hIcon = LoadIcon( PuebloDLL.hModule,
										MAKEINTRESOURCE( suIconID ) );
		#endif
		classStruct.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

		sprintf( buffer, CLASS_PANE_FRAME, m_luClassNumber );
		classStruct.lpszClassName = buffer;
		m_luClassNumber++;

		boolClassCreated = (AfxRegisterClass( &classStruct ) != FALSE);
		if (!boolClassCreated)
		{										// Return empty on error
			buffer[0] = 0;
		}

		return ChString(buffer);
	}
	#else	// defined( CH_MSW )
	{
		cerr << "Not implemented. " << __FILE__ << ":" << __LINE__ << endl;
		return "";
	}
	#endif	// defined( CH_MSW )
}


/*----------------------------------------------------------------------------
	ChPane class
----------------------------------------------------------------------------*/

ChSplitter* ChPane::GetSplitter()
{ 
	return m_pPaneMgr->GetCore()->GetSplitter(); 
}


ChWnd* ChPane::GetFrameWnd()
{
	if ((0 == m_pFrame) &&
		((paneText == GetType()) || (paneGraphic == GetType())))
	{
		m_pFrame = (ChWnd*)GetSplitter();
	}
	else if (0 == m_pFrame)
	{										// Time to create the frame
		CreateNewFrame();

		ASSERT( m_pFrame );
	}

	return m_pFrame;
}


void ChPane::GetSize( chint16& sWidth, chint16& sHeight )
{
	ChWnd*		pWnd = GetWindow();

	if (0 == pWnd)
	{
		sWidth = 0;
		sHeight = 0;
	}
	else
	{
		#if defined( CH_MSW )
		{
			CRect	rtClient;

			GetFrameWnd()->GetClientRect( rtClient );

			sWidth = rtClient.Width();
			sHeight = rtClient.Height();
		}
		#else
		{
			TRACE( "Platform not defined in ChPane::GetSize()\n" );
		}
		#endif
	}
}


ChPane* ChPane::SetOwner( const ChModuleID& idModule, ChWnd* pChild,
							ChPaneWndMethods* pChildMethods,
							chparam userData )
{
	MakeNewOwner( idModule, pChild, pChildMethods, true, userData );

	return this;
}


ChPane* ChPane::SetOwner( const ChModuleID& idModule, ChWnd* pChild,
							ChPaneWndMethods* pChildMethods )
{
	MakeNewOwner( idModule, pChild, pChildMethods );

	return this;
}


ChPane* ChPane::SetSize( chint16 sWidth, chint16 sHeight )
{
	ChWnd*		pWnd = GetFrameWnd();

	m_sWidth = sWidth;
	m_sHeight = sHeight;

	m_boolSized = true;

	if (pWnd && (GetType() == paneNormal))
	{
		#if defined( CH_MSW )
		{
			if (GetOptions() & paneInternal)
			{
				((ChSplitterBanner*)GetFrameWnd())->SetChildSize( sWidth,
																	sHeight );
			}
			else
			{
				CRect	rtFrame;
				CRect	rtClient( 0, 0, sWidth, sHeight );
				int		iHeight;
				int		iWidth;

				pWnd->GetWindowRect( rtFrame );
				pWnd->CalcWindowRect( &rtClient );
				iWidth = rtClient.Width();
				iHeight = rtClient.Height();

				if (iWidth + rtFrame.left > m_iScreenWidth)
				{
					iWidth = m_iScreenWidth - rtFrame.left;
				}

				if (iHeight + rtFrame.top > m_iScreenHeight)
				{
					iHeight = m_iScreenHeight - rtFrame.top;
				}

				pWnd->SetWindowPos( 0, 0, 0, iWidth, iHeight,
									SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
			}
		}
		#else
		{
			TRACE( "Platform not defined in ChPane::SetSize()\n" );
		}
		#endif
	}

	return this;
}


ChPane* ChPane::SetTitle( const ChString& strTitle )
{
	ChWnd*		pFrame = GetFrameWnd();
											// Save internally
	m_strTitle = strTitle;

	if (0 != pFrame )
	{										/* The window exists, so set the
												title */
		#if defined( CH_MSW )
		{
			pFrame->SetWindowText( strTitle );
		}
		#else
		{
			TRACE( "Platform not defined in ChPane::SetTitle()\n" );
		}
		#endif
	}

	return this;
}


ChPane* ChPane::Show( bool boolShow )
{
	ChWnd*		pFrame = GetFrameWnd();
											// Make sure this is a boolean
	boolShow = !!boolShow;

	if (boolShow != IsShown())
	{
		if (paneNormal == GetType())
		{
											// Shown state is changing

			if (boolShow && (0 == pFrame))
			{								/* It's finally time to create
												the frame */
				CreateNewFrame();

				ASSERT( 0 != GetFrameWnd() );
			}

			if (boolShow && !m_boolHasBeenShown)
			{
				OnInitialShow();
				m_boolHasBeenShown = true;
			}
											// Now do the show / hide
			#if defined( CH_MSW )
			{
				pFrame->ShowWindow( boolShow ? SW_SHOW : SW_HIDE );
			}
			#else
			{
				TRACE( "Platform not defined in ChPane::Show()\n" );
			}
			#endif
		}
		else
		{									// Splitter pane

			int			iIndex = (paneText == GetType()) ? SPLIT_PANE_TEXT :
															SPLIT_PANE_GRAF;
			ChSplitter*	pSplitter = GetSplitter();

			ASSERT( 0 != pSplitter );

			if (boolShow && !m_boolHasBeenShown)
			{
				OnInitialShow();
				m_boolHasBeenShown = true;
			}

			pSplitter->Show( iIndex, boolShow );
		}
	}
											// Save curr state internally
	m_boolShown = boolShow;

	return this;
}


ChPane* ChPane::SizeToFit()
{
	if (IsSizeToFit() && m_pChildMethods)
	{
		ChSize		size( m_sWidth, m_sHeight );

											// Perform the initial size-to-fit
		m_pChildMethods->GetIdealSize( size );
		SetSize( (chint16)size.cx, (chint16)size.cy );
	}

	return this;
}


void ChPane::GetSizePrefs( chint16& sIdealWidth, chint16& sIdealHeight,
							chint16& sMinWidth, chint16& sMinHeight )
{
	if (paneNormal == GetType())
	{
		if (GetFrameWnd())
		{
			#if defined( CH_MSW )
			{
				ChRect	rtClient;

				GetFrameWnd()->GetClientRect( &rtClient );
				sIdealWidth = rtClient.Width();
				sIdealHeight = rtClient.Height();
			}
			#else	// defined( CH_MSW )
			{
				cerr << "ChPane::GetSizePrefs()" << endl;
			}
			#endif	// defined( CH_MSW )
		}
		else
		{
			sIdealWidth = m_sIdealWidth;
			sIdealHeight = m_sIdealHeight;
		}

		sMinWidth = m_sMinWidth;
		sMinHeight = m_sMinHeight;
	}
	else
	{
		int			iIndex;
		ChSplitter*	pSplitter = GetSplitter();

		iIndex = (paneText == GetType()) ? SPLIT_PANE_TEXT : SPLIT_PANE_GRAF;
		pSplitter->GetPaneInfo( iIndex, sIdealWidth, sMinWidth,
								sIdealHeight, sMinHeight );
	}
}


void ChPane::SetSizePrefs( chint16 sIdealWidth, chint16 sIdealHeight,
							chint16 sMinWidth, chint16 sMinHeight )
{
	m_sIdealWidth = sIdealWidth;
	m_sIdealHeight = sIdealHeight;
	m_sMinWidth = sMinWidth;
	m_sMinHeight = sMinHeight;

	if ((paneText == GetType()) || (paneGraphic == GetType()))
	{
		int			iIndex;
		ChSplitter*	pSplitter = GetSplitter();

		iIndex = (paneText == GetType()) ? SPLIT_PANE_TEXT : SPLIT_PANE_GRAF;
		pSplitter->SetPaneInfo( iIndex, sIdealWidth, sMinWidth,
								sIdealHeight, sMinHeight );
	}
	else if (GetOptions() & paneInternal)
	{										/* Set the banner child size to
												the ideal size */
		ChSplitterBanner*	pBanner;

		pBanner = (ChSplitterBanner*)GetFrameWnd();
		pBanner->SetChildSize( sIdealWidth, sIdealHeight );
	}
}


/*----------------------------------------------------------------------------
	This method will search a HTML string for a Pane attribute.  It is
	a shortcut to the GetHTMLAttribute method.
----------------------------------------------------------------------------*/

#if defined( CH_MSW )

bool ChPane::GetPaneCmdAttr( const char* pstrArgs, ChPaneCmd& paneCommand )
{
	bool	boolFound;
	ChString	strPaneCommand;

	if (boolFound = ChHtmlWnd::GetHTMLAttribute( pstrArgs, ATTR_PANE,
													strPaneCommand ))
	{
		strPaneCommand.MakeLower();
		if ("open" == strPaneCommand)
		{
			paneCommand = paneOpen;
		}
		else if ("close" == strPaneCommand)
		{
			paneCommand = paneClose;
		}
		else
		{
			boolFound = false;
		}
	}

	return boolFound;
}

#endif	// defined( CH_MSW )

/*----------------------------------------------------------------------------
	This method will search Pane options attribute.  It is a shortcut to
	the GetHTMLAttribute method.
----------------------------------------------------------------------------*/

bool ChPane::GetPaneOptionsAttr( const char* pstrArgs, chflag32& flOptions )
{
	bool	boolFound;
	ChString	strPaneOptions;

	flOptions = 0;

	if (boolFound = ChHtmlWnd::GetHTMLAttribute( pstrArgs, ATTR_PANE_OPTIONS,
													strPaneOptions ))
	{
		if (!strPaneOptions.IsEmpty())
		{
			int		iOptionsCount = sizeof( paneOptionsList ) /
										sizeof( PaneOptionsNameType );
			char*	pstrCopy;
			char*	pstrToken;

			strPaneOptions.MakeLower();

			pstrCopy = new char[strPaneOptions.GetLength() + 2];
			strcpy( pstrCopy, strPaneOptions );

			pstrToken = strtok( pstrCopy, PANE_OPTIONS_DELIM );
			while (pstrToken)
			{
				bool	boolFound = false;
				int		iLoop;
				ChString	strTok( pstrToken );

				for (iLoop = 0; (iLoop < iOptionsCount) && !boolFound;
						iLoop++)
				{
					if (strTok == paneOptionsList[iLoop].pstrName)
					{
						flOptions |= paneOptionsList[iLoop].flOption;
						boolFound = true;
					}
				}

				pstrToken = strtok( 0, PANE_OPTIONS_DELIM );
			}

			delete [] pstrCopy;
		}
	}

	return boolFound;
}


/*----------------------------------------------------------------------------
	This method will search a HTML string for a Pane name attribute.  It
	is a shortcut to the GetHTMLAttribute method.
----------------------------------------------------------------------------*/

bool ChPane::GetPaneNameAttr( const char* pstrArgs, ChString& strPaneName )
{
	bool	boolFound;

	boolFound = ChHtmlWnd::GetHTMLAttribute( pstrArgs, ATTR_NAME,
												strPaneName );
	if (!boolFound)
	{
		strPaneName = "";
	}

	return boolFound;
}


/*----------------------------------------------------------------------------
	This method will search a HTML string for a Pane size attributes.  It
	is a shortcut to the GetHTMLAttribute method.

	If an attribute is not found, then the value remains unchanged.
----------------------------------------------------------------------------*/

void ChPane::GetPaneSizeAttrs( const char* pstrArgs, chint16& sWidth,
								chint16& sHeight, chint16& sMinWidth,
								chint16& sMinHeight )
{
	ChString	strNum;

	if (ChHtmlWnd::GetHTMLAttribute( pstrArgs, ATTR_WIDTH, strNum ))
	{
		sWidth = (chint16)atoi( strNum );
	}

	if (ChHtmlWnd::GetHTMLAttribute( pstrArgs, ATTR_HEIGHT, strNum ))
	{
		sHeight = (chint16)atoi( strNum );
	}

	if (ChHtmlWnd::GetHTMLAttribute( pstrArgs, ATTR_MIN_WIDTH, strNum ))
	{
		sMinWidth = (chint16)atoi( strNum );
	}

	if (ChHtmlWnd::GetHTMLAttribute( pstrArgs, ATTR_MIN_HEIGHT, strNum ))
	{
		sMinHeight = (chint16)atoi( strNum );
	}
}


/*----------------------------------------------------------------------------
	This method will search a HTML string for a Pane border size attributes.
	It is a shortcut to the GetHTMLAttribute method.

	If an attribute is not found, then the value remains unchanged.
----------------------------------------------------------------------------*/

void ChPane::GetPaneBorderAttrs( const char* pstrArgs, chint16& sHSpace,
									chint16& sVSpace )
{
	ChString	strNum;

	if (ChHtmlWnd::GetHTMLAttribute( pstrArgs, ATTR_HSPACE, strNum ))
	{
		sHSpace = (chint16)atoi( strNum );
	}

	if (ChHtmlWnd::GetHTMLAttribute( pstrArgs, ATTR_VSPACE, strNum ))
	{
		sVSpace = (chint16)atoi( strNum );
	}
}


/*----------------------------------------------------------------------------
	This method will search a HTML string for a Pane attribute.  It is
	a shortcut to the GetHTMLAttribute method.

	If a pane command is found, then the pane name and title will also
	be returned.
----------------------------------------------------------------------------*/

#if defined( CH_MSW )

bool ChPane::GetPaneAttrs( const char* pstrArgs, ChPaneCmd& paneCommand,
							ChString& strPaneName, ChString& strPaneTitle,
							chflag32& flOptions )
{
	bool	boolFound;
	ChString	strPaneCommand;

	flOptions = 0;

	if (boolFound = GetPaneCmdAttr( pstrArgs, paneCommand ))
	{
		GetPaneNameAttr( pstrArgs, strPaneName );

		if (!ChHtmlWnd::GetHTMLAttribute( pstrArgs, ATTR_PANE_TITLE,
											strPaneTitle ))
		{
			strPaneTitle = "";
		}

		GetPaneOptionsAttr( pstrArgs, flOptions );
	}

	if (0 == flOptions)
	{
		flOptions = defPaneOptions;
	}

	return boolFound;
}

#endif	// defined( CH_MSW )


void ChPane::OnFrameDestroy( const ChRect& rtFrame )
{
	bool	boolSaveFramePos;
											/* Only save the size if the frame
												is persistant and named */
	boolSaveFramePos = IsPersistent() && IsNamed();

											/* Only save the size if the pane
												is external (this may be 
												changed later) */

	boolSaveFramePos = boolSaveFramePos && !IsInternal();

	if (boolSaveFramePos)
	{										// Save the size
		ChRegistry		reg( CH_LAYOUT_GROUP );
		char			buffer[128];
		chint16			sWidth;
		chint16			sHeight;
		ChString			strRegName( GetName() );

		GetSize( sWidth, sHeight );
											/* Write out the data in the form:
													left of frame,
													top of frame,
													width, height */
		sprintf( buffer, "(%hd, %hd, %hd, %hd)",
					(chint16)rtFrame.left, (chint16)rtFrame.top,
					sWidth, sHeight );
		strRegName = "*" + strRegName;
		reg.Write( strRegName, buffer );
	}
											// Trigger a disconnect
	SetOwner( 0, 0, 0 );
}


/*----------------------------------------------------------------------------
	ChPane private static value
----------------------------------------------------------------------------*/

chuint32	ChPane::m_luPaneNumber = 0;
int			ChPane::m_iScreenWidth = 0;
int			ChPane::m_iScreenHeight = 0;


/*----------------------------------------------------------------------------
	ChPane protected methods
----------------------------------------------------------------------------*/

ChPane::ChPane( const ChString& strName, ChPaneMgr* pPaneMgr,
				chint16 sIdealWidth, chint16 sIdealHeight, chparam userData,
				chflag32 flOptions ) :
		m_type( paneNormal ),
		m_pPaneMgr( pPaneMgr ),
		m_strName( strName ),
		m_userData( userData ),
		m_flOptions( flOptions ),
		m_sIdealWidth( sIdealWidth ),
		m_sIdealHeight( sIdealHeight )
{
	Construct();
}
											/* This constructor is for
												registering fixed, 'special'
												panes */

ChPane::ChPane( PaneType type, const ChString& strName, ChPaneMgr* pPaneMgr ) :
		m_type( type ),
		m_pPaneMgr( pPaneMgr ),
		m_strName( strName ),
		m_userData( 0 ),
		m_flOptions( 0 ),
		m_sIdealWidth( 200 ),				// These don't matter
		m_sIdealHeight( 200 )
{
	Construct();
}


ChPane::~ChPane()
{
	#if defined( CH_MSW )
	{
		if (paneNormal == GetType())
		{
			if (m_pFrame)
			{
				if (GetOptions() & paneInternal)
				{
					ChSplitter*	pSplitter = GetSplitter();

												// Destroy the banner

					pSplitter->DestroyBanner( m_pFrame );
				}
				else
				{
					m_pFrame->DestroyWindow();	/* Automatically deletes object
													for CMiniFrameWnd objects */

					if (!(GetOptions() & paneSmallTitle))
					{
						delete m_pFrame;
					}
				}
			}
		}
	}
	#else	// defined( CH_MSW )
	{
		cerr << "ChPane::~ChPane" << endl;
	}
	#endif	// defined( CH_MSW )
}


void ChPane::Construct()
{
	m_boolShown = false;
	m_pFrame = 0;
	m_pChild = 0;
	m_pChildMethods = 0;
	m_idModule = 0;
	m_sLeft = 10;
	m_sTop = 10;
	m_sWidth = 0;
	m_sHeight = 0;
	m_sMinWidth = 20;
	m_sMinHeight = 20;
	m_boolHasBeenShown = false;
	m_boolSized = false;

	m_boolNamed = !GetName().IsEmpty();

	if ((0 == m_iScreenWidth) || (0 == m_iScreenHeight))
	{
		m_iScreenWidth = GetSystemMetrics( SM_CXSCREEN );
		m_iScreenHeight = GetSystemMetrics( SM_CYSCREEN );
	}

	if (GetType() != paneNormal)
	{
		m_flOptions |= paneFrame;
	}

	if (!IsNamed())
	{										// Make up a unique name
		do
		{
			char	buffer[32];

			sprintf( buffer, "%lu", m_luPaneNumber );
			m_strName = "Pueblo unique pane number ";
			m_strName += buffer;

			m_luPaneNumber++;

		} while (0 != GetPaneMgr()->FindPane( m_strName ));
	}
	else									// Frame *is* named
	{
		bool	boolRestoreFramePos;
											/* Only restore the size if the
												frame is persistant and
												sizeable */

		boolRestoreFramePos = IsPersistent() && IsSizeable();

											/* Don't restore the size if
												'force' is specified */

		boolRestoreFramePos = boolRestoreFramePos &&
								!(GetOptions() & paneForce);

											/* Only restore the size if the
												pane is external (this may be
												changed later) */

		boolRestoreFramePos = boolRestoreFramePos && !IsInternal();

		if (boolRestoreFramePos)
		{									/* If the pane is named, it may
												have a previous size stored
												away.  Try to get it. */
			ChRegistry		reg( CH_LAYOUT_GROUP );
			ChString			strRegData;
			ChString			strRegName( GetName() );

			strRegName = "*" + strRegName;
			if (reg.Read( strRegName, strRegData, "" ))
			{
				chint16		sLeft;
				chint16		sTop;
				chint16		sWidth;
				chint16		sHeight;
											/* Read the data in the form:
													left of frame,
													top of frame,
													width of content,
													height of content */

				if (4 == sscanf( strRegData, "(%hd, %hd, %hd, %hd)",
									&sLeft, &sTop, &sWidth, &sHeight ))
				{
					m_sLeft = sLeft;
					m_sTop = sTop;

					if (sWidth)
					{
						m_sWidth = m_sIdealWidth = sWidth;
					}

					if (sHeight)
					{
						m_sHeight = m_sIdealHeight = sHeight;
					}

					m_boolSized = true;
				}
			}
		}
	}
}


void ChPane::OnInitialShow()
{
											/* Perform the initial size-to-fit,
												if appropriate */
	SizeToFit();
}


void ChPane::MakeNewOwner( const ChModuleID& idNewModule, ChWnd* pChild,
							ChPaneWndMethods* pChildMethods,
							bool boolChangeUserData, chparam userData )
{
	if (m_idModule && (m_idModule != idNewModule))
	{
											/* Notify the old owner that a new
												kid is taking over this turf */
		OnFrameDisconnect( idNewModule );
	}
											// Store the new data
	m_pChild = pChild;
	m_pChildMethods = pChildMethods;
	m_idModule = idNewModule;

	if (boolChangeUserData)
	{
		m_userData = userData;
	}
											// Tell the frame to resize itself
	if (paneNormal == GetType())
	{
		ChWnd*		pFrame = GetFrameWnd();

		if (0 != pFrame)
		{
			#if defined( CH_MSW )
			{
				CRect	rtClient;

				pFrame->GetClientRect( rtClient );

				if (GetOptions() & paneSmallTitle)
				{
					((ChPaneMiniFrame*)pFrame)->SizeChild( rtClient );
				}
				else if (GetOptions() & paneInternal)
				{
					((ChSplitterBanner*)pFrame)->SetChild( pChild );
				}
				else
				{
					((ChPaneFrame*)pFrame)->SizeChild( rtClient );
				}
			}
			#endif	// defined( CH_MSW )
		}
	}
	else
	{										// Splitter pane

		int			iIndex = (paneText == GetType()) ? SPLIT_PANE_TEXT :
														SPLIT_PANE_GRAF;
		ChSplitter*	pSplitter = GetSplitter();

		if (0 == pChild)
		{
			pSplitter->EmptyPane( iIndex );
		}
		else
		{
			ASSERT( 0 != pSplitter );
											// Set the window into the pane

			pSplitter->SetWnd( iIndex, pChild );
		}
	}
}


void ChPane::CreateNewFrame()
{
	chint16		sWidth = m_sWidth ? m_sWidth : m_sIdealWidth;
	chint16		sHeight = m_sHeight ? m_sHeight : m_sIdealHeight;

	#if defined( CH_MSW )
	{
		if (GetOptions() & paneInternal)
		{
			ChSplitter*			pSplitter = GetSplitter();
			ChSplitterBanner*	pBanner;
			bool				boolTop;
			bool				boolFrame = !!(GetOptions() & paneFrame);

			// UE: ensure internal panes don't have small titles - that really
			//     screws things up otherwise.
			m_flOptions &= ~paneSmallTitle;
			
			boolTop = !(paneAlignBottom & GetOptions());
			pBanner = pSplitter->CreateBanner( this, GetName(),
												boolTop, boolFrame,
												sWidth, sHeight );
			m_pFrame = pBanner;
		}
		else
		{
			DWORD		dwStyle = WS_BORDER | WS_CAPTION | WS_SYSMENU |
									WS_MINIMIZEBOX;
			DWORD		dwExStyle = 0;
			ChString		strClassName;
			HWND		hwndParent;
			CRect		rtFrame;
			chuint16	suIconID;
			bool		boolCloseable;

			if (GetOptions() & paneSmallTitle)
			{
				m_pFrame = new ChPaneMiniFrame( GetPaneMgr(), this );
			}
			else
			{
				m_pFrame = new ChPaneFrame( GetPaneMgr(), this );
			}

			ASSERT( 0 != GetFrameWnd() );

			if (GetOptions() & paneFloat)
			{
				dwStyle |= WS_POPUP;
				dwExStyle |= WS_EX_TOPMOST;

				hwndParent = GetPaneMgr()->GetCore()->GetFrameWnd()->m_hWnd;
			}
			else
			{
				dwStyle |= WS_OVERLAPPED;

				hwndParent = 0;
			}

			if (GetOptions() & paneSizeable)
			{
				dwStyle |= WS_THICKFRAME | WS_MAXIMIZEBOX;
			}

			suIconID = GetPaneIconID();
			boolCloseable = !!(GetOptions() & paneCloseable);
			strClassName = GetPaneMgr()->FindClassName( boolCloseable,
														suIconID );
			if (!strClassName.IsEmpty())
			{
				GetFrameWnd()->CreateEx( dwExStyle, strClassName, GetTitle(),
											dwStyle, m_sLeft, m_sTop,
											sWidth, sHeight, hwndParent, 0 );

				if (!IsSizeToFit())
				{							// Size according to content
					chint16		sWidthDiff;
					chint16		sHeightDiff;
					ChRect		rtClient;

					GetFrameWnd()->GetClientRect( &rtClient );
					GetFrameWnd()->GetWindowRect( &rtFrame );
					sWidthDiff = rtFrame.Width() - rtClient.Width();
					sHeightDiff = rtFrame.Height() - rtClient.Height();

					GetFrameWnd()->SetWindowPos( 0, 0, 0, sWidth + sWidthDiff,
													sHeight + sHeightDiff,
													SWP_NOACTIVATE |
														SWP_NOMOVE |
														SWP_NOZORDER );
				}
			}
		}
	}
	#else
	{
		TRACE( "Platform not defined in ChPane::CreateNewFrame()\n" );
	}
	#endif
}


chuint16 ChPane::GetPaneIconID()
{
	chuint16	suIconID;
	ChString		strPaneName = GetName();

	strPaneName.MakeLower();
	if (strPaneName == "scroll")
	{
		suIconID = IDI_SCROLL;
	}
	else if (strPaneName == "map")
	{
		suIconID = IDI_MAP;
	}
	else if (strPaneName == "book")
	{
		suIconID = IDI_BOOK;
	}
	else if (strPaneName == "log")
	{
		suIconID = IDI_BOOK_LOG;
	}
	else
	{										/* Look for keywords in the title
												(This is not great since it
												doesn't look for delimited
												keywords.) */
		strPaneName = GetTitle();

		strPaneName.MakeLower();
		if (-1 != strPaneName.Find( "map" ))
		{
			suIconID = IDI_MAP;
		}
		else if (-1 != strPaneName.Find( "scroll" ))
		{
			suIconID = IDI_SCROLL;
		}
		else if (-1 != strPaneName.Find( "log" ))
		{
			suIconID = IDI_BOOK_LOG;
		}
		else
		{									// Default to book
			suIconID = IDI_BOOK;
		}
	}

	return suIconID;
}


bool ChPane::ProcessClose()
{
											/* Notify the child window the
												frame is about to close */
	return OnFrameClose();
}


/*----------------------------------------------------------------------------
	ChPaneWindowClass class
----------------------------------------------------------------------------*/

ChPaneWindowClass::ChPaneWindowClass( const ChString& strName,
										bool boolCloseable,
										chuint16 suIconID ) :
				m_strName( strName ),
				m_boolCloseable( boolCloseable ),
				m_suIconID( suIconID )
{
}


#if defined( CH_MSW )

/*----------------------------------------------------------------------------
	ChPaneFrame class
----------------------------------------------------------------------------*/

ChPaneFrame::ChPaneFrame( ChPaneMgr* pPaneMgr, ChPane* pPane ) :
				m_pPane( pPane ),
				m_pPaneMgr( pPaneMgr ),
				m_iFrameWidth( 0 ),
				m_iFrameHeight( 0 )
{
}


ChPaneFrame::~ChPaneFrame()
{
}


void ChPaneFrame::SizeChild( CRect& rtClient )
{
	if (GetPane()->GetWindow())
	{
		CPoint	ptTopLeft = rtClient.TopLeft();

		GetPane()->GetWindow()->SetWindowPos( 0, ptTopLeft.x, ptTopLeft.y,
												rtClient.Width(),
												rtClient.Height(),
												SWP_NOACTIVATE | SWP_NOZORDER );
	}
}


BEGIN_MESSAGE_MAP(ChPaneFrame, CWnd)
	//{{AFX_MSG_MAP(ChPaneFrame)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_WM_GETMINMAXINFO()
	ON_WM_DESTROY()
	ON_WM_PALETTECHANGED()
	ON_WM_QUERYNEWPALETTE()
	//}}AFX_MSG_MAP
	ON_MESSAGE( WM_CTLCOLORDLG, OnCtlBkColor )
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChPaneFrame message handlers
----------------------------------------------------------------------------*/

void ChPaneFrame::OnSize( UINT nType, int iX, int iY )
{
	CRect	rtClient;

	CWnd::OnSize( nType, iX, iY );
											/* Notify the child that the
												content area is changing */
	if (GetPane()->OnFrameSize( iX, iY ))
	{
		if (SIZE_MINIMIZED != nType)
		{									/* Size the child as long as we're
												not being minimized */
			GetClientRect( rtClient );
			rtClient.InflateRect( 1, 1 );
			SizeChild( rtClient );
		}
	}
}


void ChPaneFrame::OnClose()
{
	ChPane*		pPane = GetPane();

	if (pPane->ProcessClose())
	{
		ChString		strName;
											/* Call default behavior
												(which destroys the window) */
		CWnd::OnClose();

		strName = pPane->GetName();
		GetPaneMgr()->DestroyPane( strName );
	}
}


void ChPaneFrame::OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI )
{
	CWnd::OnGetMinMaxInfo( lpMMI );

	if ((0 == m_iFrameWidth) && (0 == m_iFrameHeight))
	{
		CRect	rtFrame;
		CRect	rtClient;

		GetWindowRect( rtFrame );
		GetClientRect( rtClient );

		m_iFrameWidth = rtFrame.Width() - rtClient.Width();
		m_iFrameHeight = rtFrame.Height() - rtClient.Height();
	}

	lpMMI->ptMinTrackSize.x = GetPane()->GetMinWidth() + m_iFrameWidth;
	lpMMI->ptMinTrackSize.y = GetPane()->GetMinHeight() + m_iFrameHeight;
}


void ChPaneFrame::OnDestroy()
{
	ChPane*		pPane = GetPane();

	if (pPane)
	{
		ChRect		rtFrame;

		GetWindowRect( &rtFrame );
		pPane->OnFrameDestroy( rtFrame );
	}
											// Call parent processing
	CWnd::OnDestroy();
}


void ChPaneFrame::OnPaletteChanged(CWnd* pFocusWnd) 
{
	SendMessageToDescendants( WM_PALETTECHANGED, 
							(WPARAM)(pFocusWnd->GetSafeHwnd()), 0, 0 );
	
}

BOOL ChPaneFrame::OnQueryNewPalette() 
{
	BOOL boolChanged = FALSE;

	HWND hWnd = GetSafeHwnd();
	// walk through HWNDs to avoid creating temporary CWnd objects
	for (HWND hWndChild = ::GetTopWindow(hWnd); 
		!boolChanged && hWndChild != NULL;
		hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT))
	{
		// send message with Windows SendMessage API
		boolChanged = (BOOL)::SendMessage(hWndChild, WM_QUERYNEWPALETTE, 
											0, 0 );
	}

	#if 0
	CPalette * pPal = ChDib::GetStdPalette();

	if ( pPal )
	{

        CDC*	pDC = GetDC();
		CPalette * pOldPal = pDC->SelectPalette( pPal, false );

        boolChanged	 = (BOOL)pDC->RealizePalette();
	
		pDC->SelectPalette( pOldPal, true );

        pDC->RealizePalette();

        ReleaseDC( pDC );
	}
	#endif


	return boolChanged;
}

LONG ChPaneFrame::OnCtlBkColor( WPARAM wParam, LPARAM lParam )
{
	return 0L;
}


/*----------------------------------------------------------------------------
	ChPaneMiniFrame class
----------------------------------------------------------------------------*/

ChPaneMiniFrame::ChPaneMiniFrame( ChPaneMgr* pPaneMgr, ChPane* pPane ) :
				m_pPaneMgr( pPaneMgr ),
				m_pPane( pPane ),
				m_iFrameWidth( 0 ),
				m_iFrameHeight( 0 )
{
}


ChPaneMiniFrame::~ChPaneMiniFrame()
{
}


void ChPaneMiniFrame::SizeChild( CRect& rtClient )
{
	if (GetPane()->GetWindow())
	{
		CPoint	ptTopLeft = rtClient.TopLeft();

		GetPane()->GetWindow()->SetWindowPos( 0, ptTopLeft.x, ptTopLeft.y,
												rtClient.Width(),
												rtClient.Height(),
												SWP_NOACTIVATE | SWP_NOZORDER );
	}
}




BEGIN_MESSAGE_MAP(ChPaneMiniFrame, CMiniFrameWnd)
	//{{AFX_MSG_MAP(ChPaneMiniFrame)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_WM_GETMINMAXINFO()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_MESSAGE( WM_CTLCOLORDLG, OnCtlBkColor )
END_MESSAGE_MAP()


/*----------------------------------------------------------------------------
	ChPaneMiniFrame message handlers
----------------------------------------------------------------------------*/

void ChPaneMiniFrame::OnSize( UINT nType, int iX, int iY )
{
	CRect	rtClient;

	CMiniFrameWnd::OnSize( nType, iX, iY );
											/* Notify the child that the
												content area is changing */
	if (GetPane()->OnFrameSize( iX, iY ))
	{
		GetClientRect( rtClient );
		rtClient.InflateRect( 1, 1 );
		SizeChild( rtClient );
	}
}


void ChPaneMiniFrame::OnClose()
{
	ChPane*		pPane = GetPane();

	if (pPane->ProcessClose())
	{
		ChString		strName = pPane->GetName();
		ChPaneMgr*	pPaneMgr = GetPaneMgr();
											/* Call default behavior
												(which destroys the window) */
		CMiniFrameWnd::OnClose();
											// Remove the pane from the manager
		pPaneMgr->DestroyPane( strName );
	}
}


void ChPaneMiniFrame::OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI )
{
	CMiniFrameWnd::OnGetMinMaxInfo( lpMMI );

	if ((0 == m_iFrameWidth) && (0 == m_iFrameHeight))
	{
		CRect	rtFrame;
		CRect	rtClient;

		GetWindowRect( rtFrame );
		GetClientRect( rtClient );

		m_iFrameWidth = rtFrame.Width() - rtClient.Width();
		m_iFrameHeight = rtFrame.Height() - rtClient.Height();
	}

	lpMMI->ptMinTrackSize.x = GetPane()->GetMinWidth() + m_iFrameWidth;
	lpMMI->ptMinTrackSize.y = GetPane()->GetMinHeight() + m_iFrameHeight;
}


void ChPaneMiniFrame::OnDestroy()
{
	ChPane*		pPane = GetPane();

	if (pPane)
	{
		ChRect		rtFrame;

		GetWindowRect( &rtFrame );
		pPane->OnFrameDestroy( rtFrame );
	}
											// Call parent processing
	CMiniFrameWnd::OnDestroy();
											/* For the CMiniFrameWnd class, the
												C++ object is deleted when the
												window is destroyed, so make
												sure that the Pane object
												knows this. */
	if (pPane)
	{
		pPane->FrameDestroyed();
	}
}


LONG ChPaneMiniFrame::OnCtlBkColor( WPARAM wParam, LPARAM lParam )
{
	return 0L;
}

/*----------------------------------------------------------------------------
	ChPaneMiniFrame window proc
----------------------------------------------------------------------------*/

LRESULT CALLBACK PaneFrameCallback( HWND hWnd, UINT uMsg,
									WPARAM wParam, LPARAM lParam )
{
	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

#endif

// $Log$
// Revision 1.1.1.1  2003/02/03 18:54:28  uecasm
// Import of source tree as at version 2.53 release.
//
