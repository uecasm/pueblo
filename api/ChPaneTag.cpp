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

	This file contains the implementation all body elements.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <ChHtmWnd.h>
#include <ChHtmlSettings.h>

#include "ChHtmlView.h"
#include "ChHtmSym.h"
#include "ChHtmlParser.h"
#include "ChHtmlPane.h"
#include "ChPaneTag.h"

#include <MemDebug.h>

typedef struct tagAttrMap
{
	
	const char* 	pstrValue;
	chuint32		luValue;

} AttrMap;

CH_INTERN_VAR AttrMap 	astrOptionMap[] =
				{
					{ "overlapped",  	ChHtmlPane::optionOverlapped },
					{ "floating",  		ChHtmlPane::optionFloating },
					{ "internal",  		ChHtmlPane::optionInternal },
					{ "docking",  		ChHtmlPane::optionDocking },
					{ "browser",  		ChHtmlPane::optionBrowser },
					{ "nonsizeable", 	ChHtmlPane::optionNonSizeable },
					{ "noclose",  		ChHtmlPane::optionNoClose },
					{ "small_title", 	ChHtmlPane::optionSmallTitle },
					{ "fit",  			ChHtmlPane::optionFit },
					{ "existing",	   	ChHtmlPane::optionExisting },
					{ "docked",  		ChHtmlPane::optionDocked },
					{ "persistent",		ChHtmlPane::optionPersistent },
					{ "force",			ChHtmlPane::optionForce },
					{ "webtracker",		ChHtmlPane::optionWebTracker },
					{ "viewbottom",		ChHtmlPane::optionViewBottom },
				};

CH_INTERN_VAR AttrMap 	astrActionMap[] =
				{
					{ "redirect",  	ChHtmlPane::actionRedirect },
					{ "open",  		ChHtmlPane::actionOpen },
					{ "close", 		ChHtmlPane::actionClose },
					{ "move",	   	ChHtmlPane::actionMove },
				};	  

CH_INTERN_VAR AttrMap 	astrScrollMap[] =
				{
					{ "yes",  		ChHtmlPane::scrollYes },
					{ "no",  		ChHtmlPane::scrollNo },
					{ "auto", 		ChHtmlPane::scrollAuto },
				};

 // 	processed in this file
 // XCH_PANE tag - 



/*----------------------------------------------------------------------------
	ChPaneTag class 
----------------------------------------------------------------------------*/

ChPaneTag::ChPaneTag( ChHtmlParser* pParser ) : ChHtmlTag( pParser )
{
	 m_iTokenID = HTML_XCHPANE;
	 m_luAttrs = attrHasArguments | attrCallProcessTag | attrTrimRight;
}


void ChPaneTag::ProcessArguments( pChArgList pList, int iArgCount )
{
	ChHtmlPane*		pPane = new ChHtmlPane();

	ASSERT( pPane );
											// set the default

	pPane->SetHSpace( GetHtmlView()->GetSettings()->GetCharWidth() );
//	pPane->SetVSpace( GetHtmlView()->GetSettings()->GetCharWidth() );

	pPane->SetWidth( GetHtmlView()->GetSettings()->GetCharWidth() * 50  );

	for (int i = 0; i < iArgCount; i++)
	{
		switch ( pList[i].iArgType )
		{
			case ARG_NAME:
			{								/* Target names are case-
												insensitive */

				ChString		strName( (const char*)pList[i].argVal );

				strName.MakeLower();
				pPane->SetName( strName );
				break;
			}

			case ARG_ACTION:
			{
				chuint32	luAction = ChHtmlPane::actionOpen;
				ChString		strAction( (const char*)pList[i].argVal );
	
				strAction.MakeLower();
	
				for (int i = 0; i < (sizeof( astrActionMap ) / sizeof( AttrMap )); i++)
				{
					if (strAction.Find( astrActionMap[i].pstrValue ) != -1)
					{
					 	luAction = astrActionMap[i].luValue;
					} 
				}

				pPane->SetAction( luAction );
				break;
			}

			case ARG_HREF:
			{
				pPane->SetURL( (const char*)pList[i].argVal );
				break;
			}

			case ARG_PANETITLE:
			{
				pPane->SetTitle( (const char*)pList[i].argVal );
				break;
			}

			case ARG_VSPACE:
			{
				pPane->SetVSpace( (int)pList[i].argVal  );
				break;
			}

			case ARG_HSPACE:
			{
				pPane->SetHSpace( (int)pList[i].argVal  );
				break;
			}

			case ARG_WIDTH:
			{
				pPane->SetWidth( (int)pList[i].argVal  );
				break;
			}

			case ARG_HEIGHT:
			{
				pPane->SetHeight( (int)pList[i].argVal  );
				break;
			}

			case ARG_MINWIDTH:
			{
				pPane->SetMinWidth( (int)pList[i].argVal  );
				break;
			}

			case ARG_MINHEIGHT:
			{
				pPane->SetMinHeight( (int)pList[i].argVal  );
				break;
			}

			case ARG_OPTIONS:
			{
				chuint32	luOption = 0;
				ChString		strOptions( (const char*)pList[i].argVal );
	
				strOptions.MakeLower();
	
				for (int i = 0; i < (sizeof( astrOptionMap )/ sizeof( AttrMap )); i++)
				{
					if (strOptions.Find( astrOptionMap[i].pstrValue ) != -1)
					{
					 	luOption |= astrOptionMap[i].luValue;
					} 
				}

				pPane->SetOptions( luOption );
				break;
			}

			case ARG_SCROLLING:
			{
				chuint32	iScroll = 0;
				ChString		strScrolling( (const char*)pList[i].argVal );
	
				strScrolling.MakeLower();
	
				for (int i = 0; i < (sizeof( astrScrollMap )/ sizeof( AttrMap )); i++)
				{
					if (strScrolling.Find( astrScrollMap[i].pstrValue ) != -1)
					{
					 	iScroll = astrScrollMap[i].luValue;	 
						break;
					} 
				}

				pPane->SetScrolling( iScroll );
				break;
			}

			case ARG_ALIGN:
			{
				pPane->SetAlign( pList[i].argVal );
				break;
			}

			case ARG_ALIGNTO:
			{
				break;
			}

			default:
			{
				break;
			}
		}
	}
											// Frame manager does the real work
	GetHtmlView()->CreatePane( pPane );
	delete pPane;
}


chint32 ChPaneTag::ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount )
{
	return GetHtmlView()->RedirectStream( pstrBuffer, lStart, lCount );
}

// $Log$
