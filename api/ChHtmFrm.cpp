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

	This file consists of the implementation of the ChHtmWnd  class.
	Formas implementation

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <ctype.h>

#ifdef CH_UNIX
#include <stdlib.h>
#include <ctype.h>
#include <ChTypes.h>
#include <ChRect.h>
#include <ChSize.h>
#include <ChScrlVw.h>
#include <ChDC.h>
#endif

#include <ChReg.h>
#include <ChConst.h>

#include <ChHtmWnd.h>
#include <ChUrlMap.h>
#include <ChHtmlSettings.h>

#include "ChHtmlView.h"
#include "ChHtmSym.h"
#include "ChHtmlParser.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

#include <MemDebug.h>



/*----------------------------------------------------------------------------
	ChHTMLForm class
----------------------------------------------------------------------------*/

ChHTMLForm::ChHTMLForm( ChHtmlView *pWnd ) :
				m_phtmlView( pWnd ),
				m_iMethod( methodGet ),
				m_pstrAction( 0 ),
			   	m_iNumControls( 0 ),
			   	m_iCtrlListSize( 0 ),
			   	m_pList( 0 )
{

	CDC*		pDC = pWnd->GetWindowDC();
	int 		iFontSize = -1 * ( pDC->GetDeviceCaps( LOGPIXELSY ) * 
						pWnd->GetSettings()->GetProportionalFontSize() / 72);

	LOGFONT		lf;
	ChMemClearStruct( &lf );

	lf.lfHeight = iFontSize;
	lf.lfWeight = FW_LIGHT;
	lf.lfCharSet = ANSI_CHARSET;
	lf.lfOutPrecision = OUT_STROKE_PRECIS;
	lf.lfClipPrecision = CLIP_STROKE_PRECIS;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfPitchAndFamily = FF_DONTCARE;
	lstrcpy(lf.lfFaceName, pWnd->GetSettings()->GetProportionalFontName());

	m_ctrlFont.CreateFontIndirect( &lf );

	CFont* pOld = pDC->SelectObject( &m_ctrlFont );

	TEXTMETRIC	tm;

	pDC->GetTextMetrics( &tm );
	m_iCharHeight = tm.tmHeight;
	m_iAvgCharWidth = tm.tmAveCharWidth;

	pDC->SelectObject( pOld );
	pWnd->ReleaseDC( pDC );

	m_iEditExtraWidth = m_iAvgCharWidth * 2;
	m_iListExtraWidth = GetSystemMetrics( SM_CXHTHUMB ) +
							(6 * GetSystemMetrics( SM_CXBORDER ));
	m_iPopupExtraWidth = GetSystemMetrics( SM_CXHTHUMB ) +
							(8 * GetSystemMetrics( SM_CXBORDER ));
									
									// OBM_CHECKBOXES
	HBITMAP hBmp = ::LoadBitmap( NULL, MAKEINTRESOURCE(32759) );
		
	if ( hBmp )
	{
		BITMAP bmpInfo;
		::GetObject( hBmp, sizeof(BITMAP), &bmpInfo );
		m_sizeCheckBox.cx = ( bmpInfo.bmWidth)/4;
		m_sizeCheckBox.cy = bmpInfo.bmHeight/3;
		::DeleteObject( hBmp );
	}

}


ChHTMLForm::~ChHTMLForm()
{
	if (m_pList)
	{
		for (int i= 0; i < m_iNumControls; i++)
		{
			#if defined( CH_MSW )
			{
				if (m_pList[i].pWnd)
				{
					//ASSERT( ::IsWindow( m_pList[i].pWnd->m_hWnd ) );
					delete m_pList[i].pWnd;
				}
			}
			#endif	// defined( CH_MSW )
		}
		delete []m_pList;
	}
}


void ChHTMLForm::AddControl( ChCtrlList& ctrlInfo )
{
	if (0 == m_pList)
	{
		m_iCtrlListSize = ChHTMLForm::ctrlListSize;
		m_pList = new ChCtrlList[m_iCtrlListSize];

		ASSERT( m_pList );
	}
	else if (m_iNumControls >= m_iCtrlListSize)
	{										// Realloc the list

		m_iCtrlListSize += ChHTMLForm::ctrlListGrowSize;
		pChCtrlList	pNew = new ChCtrlList[m_iCtrlListSize];
		ASSERT( m_pList );

		ChMemCopy( pNew, m_pList, (sizeof(ChCtrlList) * m_iNumControls) );
		delete [] m_pList;
		m_pList = pNew;
	}
	m_pList[m_iNumControls++] = ctrlInfo;
}

void ChHTMLForm::EscapeSpecialChars( ChString& strData )
{
	ChString strTmp;
	int i = 0;
	while( i < strData.GetLength() )
	{

		if ( strData[i] == TEXT( ' ' ) )
		{
			strTmp += TEXT( '+' );
		}
	//	else if ( (strData[i] == TEXT( '\r' )  || strData[i] == TEXT( '\n' ))  
	//			  && GetMethod() == methodPost )
	//	{ // do  not encode newlines if method is post
	//		strTmp += strData[i];
	//	}
		else if ( !isalnum( strData[i] ) )
		{
			char	strNum[10];

			strTmp += TEXT( '%' );
			wsprintf( strNum, "%02X", (int)strData[i] );
			strTmp += strNum;
		}
		else
		{
			strTmp += strData[i];
		}
		i++;
	}
	strData = strTmp;
}

void ChHTMLForm::AddNameValuePair( const char* pstrName, ChString& strVlaue )
{

  	if ( !strVlaue.IsEmpty() )
	{
		EscapeSpecialChars( strVlaue );
	}

	if( !m_strFormData.IsEmpty() )
	{
		m_strFormData += '&';
	}

	m_strFormData += pstrName;
	m_strFormData += TEXT( '=' );
	m_strFormData += strVlaue;


}

#if defined( CH_MSW )

void ChHTMLForm::SubmitForm()
{

	ChString strURL;

	if ( GetMethod() != methodXCHCMD  )
	{
		if ( !m_pstrAction )
		{
			// Set the default action to the current document URL
			strURL = GetHtmlView()->GetDocURL();
		}
		else 
		{
			ChURLParts urlParts;

			urlParts.GetURLParts( m_pstrAction, GetHtmlView()->GetDocURL() );

			strURL = urlParts.GetURL();
		
		}
	}

 	GetFormData();
	// post or get the URL

	switch ( GetMethod() )
	{
		case methodPost:
		{
			GetHtmlView()->GetFrameMgr()->PostURL( strURL, m_strFormData );
			break;
		}
		case methodXCHCMD:
		{
			ChString strAction( m_pstrAction ? m_pstrAction : TEXT( "" ) );

			// call the virtual function to handle the form submit
			// default behavior is to do nothing
			GetHtmlView()->NotifySubmitForm( strAction, m_strMD5, m_strFormData );

			#if 0
			// disable all controls in the form
			for ( int i= 0; i < m_iNumControls; i++ )
			{
				#ifdef CH_MSW
				if ( m_pList[i].pWnd )
				{
					m_pList[i].pWnd->EnableWindow( false );
				}
				#endif
			}
			#endif
			break;
		}
		default:
		{
			strURL += TEXT( '?' );
			strURL += m_strFormData;
			GetHtmlView()->GetFrameMgr()->LoadURL( strURL );
		}
	}

}

void ChHTMLForm::GetFormData()
{	// package the data to be transmitted

	m_strFormData = "";

	for ( int i = 0; i < m_iNumControls;  i++ )
	{
		switch( m_pList[i].iType )
		{
			case VAL_SUBMIT :
			{
				ChString strData;
				if ( m_pList[i].boolDefState && m_pList[i].pstrName && m_pList[i].pstrValue )
				{
					strData = m_pList[i].pstrValue;
					AddNameValuePair( m_pList[i].pstrName, strData);
				}
				break;
			}
			case VAL_HIDDEN:
			{
				ChString strData;
				if ( m_pList[i].pstrValue )
				{
					strData = m_pList[i].pstrValue;
				}
				AddNameValuePair( m_pList[i].pstrName, strData );
				break;
			}
			case VAL_CHECKBOX:
			case VAL_RADIO:
			{
				if ( m_pList[i].pstrName && m_pList[i].pWnd->SendMessage( BM_GETCHECK ) )
				{
					ChString strData;

					if ( m_pList[i].pstrValue )
					{
						strData = m_pList[i].pstrValue;
					}
					AddNameValuePair( m_pList[i].pstrName, strData );
				}
				break;
			}
			case VAL_TEXT:
			case VAL_PASSWORD:
			case TYPE_MULTILINETEXT:
			{

				if ( m_pList[i].pstrName )
				{
					ChString strData;
					ChString strTmp;

					m_pList[i].pWnd->GetWindowText( strData );

					AddNameValuePair( m_pList[i].pstrName, strData );

				}
				break;
			}

			case TYPE_POPUPLIST:
			{
				if (m_pList[i].pstrName)
				{
					ChFrmCombo*		pSelect = (ChFrmCombo*)m_pList[i].pWnd;
					ChString			strData;
					int				iSel;

					if ((iSel = pSelect->GetCurSel()) != CB_ERR)
					{
						if (m_pList[i + iSel + 1].pstrValue)
						{
							strData = m_pList[i + iSel + 1].pstrValue;
						}
						else
						{
							if ( m_pList[i].pstrValue )
							{
								strData = m_pList[i].pstrValue;
							}
							else
							{
								pSelect->GetLBText( iSel, strData );
							}
						}
					}

					AddNameValuePair( m_pList[i].pstrName, strData );
				}
				break;
			}

			case TYPE_LIST:
			{
				if ( m_pList[i].pstrName )
				{
					ChFrmList *pSelect = (ChFrmList*)m_pList[i].pWnd;
					ChString strData;

					int iSel;
					if( (iSel = pSelect->GetCurSel()) != LB_ERR )
					{
						if ( m_pList[i + iSel + 1].pstrValue )
						{
							strData = m_pList[i + iSel + 1].pstrValue;
						}
						else
						{
							if ( m_pList[i].pstrValue ) 
							{
								strData = m_pList[i].pstrValue;
							}
							else
							{
								pSelect->GetText( iSel, strData );
							}
						}
					}
					AddNameValuePair( m_pList[i].pstrName, strData );
				}
			}
			case TYPE_LISTMULTI:
			{
				if ( m_pList[i].pstrName )
				{
					ChFrmList *pSelect = (ChFrmList*)m_pList[i].pWnd;
					ChString strData;


					int iSelCount = pSelect->GetSelCount();
					if( iSelCount && iSelCount != LB_ERR )
					{
						int *pSelItems = new int [iSelCount];

						pSelect->GetSelItems( iSelCount, pSelItems );

						for ( int j = 0; j < iSelCount; j++ )
						{
							strData = TEXT( "" );

							if ( m_pList[i + pSelItems[j] + 1].pstrValue )
							{
								strData = m_pList[i + pSelItems[j] + 1].pstrValue;
							}
							else
							{
								pSelect->GetText( pSelItems[j], strData );
							}
							AddNameValuePair( m_pList[i].pstrName, strData );
						}
						delete []pSelItems;
					}

				}
				break;
			}
		}
	}
}


void ChHTMLForm::ResetForm()
{

	for ( int i = 0; i < m_iNumControls;  i++ )
	{
		switch( m_pList[i].iType )
		{
			case VAL_CHECKBOX:
			case VAL_RADIO:
			{
				m_pList[i].pWnd->SendMessage( BM_SETCHECK,
									m_pList[i].boolDefState );
				break;
			}
			case VAL_TEXT:
			case VAL_PASSWORD:
			case TYPE_MULTILINETEXT:
			{
				if ( m_pList[i].pstrValue )
				{
					m_pList[i].pWnd->SetWindowText( m_pList[i].pstrValue );
				}
				else
				{
					m_pList[i].pWnd->SetWindowText( "" );
				}
				break;
			}

			case TYPE_POPUPLIST:
			case TYPE_LIST:
			{
				int		j = 1;

				while (( i + j ) < m_iNumControls)
				{
					if (m_pList[i + j ].iType == TYPE_LIST_ELEMENT_SEL)
					{
						m_pList[i].pWnd->
							SendMessage( (m_pList[i].iType == TYPE_POPUPLIST ?
												CB_SETCURSEL:
												LB_SETCURSEL ), j - 1 );
					}
					else if (m_pList[i + j ].iType != TYPE_LIST_ELEMENT)
					{
						break;
					}
					j++;
				}
				break;
			}

			case TYPE_LISTMULTI:
			{

				int j = 1;
				m_pList[i].pWnd->SendMessage( LB_SETSEL, false, (LPARAM)-1 );
				while ( ( i + j ) < m_iNumControls )
				{
					if ( m_pList[i + j].iType == TYPE_LIST_ELEMENT_SEL )
					{
						m_pList[i].pWnd->SendMessage( LB_SETSEL, true, j - 1 );
					}
					else if ( m_pList[i + j].iType != TYPE_LIST_ELEMENT	)
					{
						break;
					}
					j++;
				}
				break;
			}
			default:
			{
				break;
			}
		}
	}

}

#endif


void ChHTMLForm::SetNextTabFocus( CWnd* pCurrent, bool boolShiftKey )
{
	int iCtlIndex = -1;
	for ( int i = 0; i < GetTotalControls(); i++ )
	{
		pChCtrlList pCtrl = GetControlInfo( i );

		if ( pCtrl->pWnd == pCurrent )
		{
			iCtlIndex = i;
			break;
		}
	}

	CWnd* pWnd = 0;

	if ( iCtlIndex != -1 )
	{
		if ( boolShiftKey )
		{
			for( int i = iCtlIndex -1; i >= 0; i-- )
			{
				pChCtrlList pCtrl = GetControlInfo( i );

				if ( pCtrl->pWnd )
				{
					pWnd = pCtrl->pWnd;
					break;
				}
			}

			if ( !pWnd )
			{
				for( int i = GetTotalControls() -1; i > iCtlIndex; i-- )
				{
					pChCtrlList pCtrl = GetControlInfo( i );

					if ( pCtrl->pWnd )
					{
						pWnd = pCtrl->pWnd;
						break;
					}
				}
			}
		}
		else
		{
			for( int i = iCtlIndex + 1; i < GetTotalControls(); i++ )
			{
				pChCtrlList pCtrl = GetControlInfo( i );

				if ( pCtrl->pWnd )
				{
					pWnd = pCtrl->pWnd;
					break;
				}
			}

			if ( !pWnd )
			{
				for( int i = 0; i < iCtlIndex; i++ )
				{
					pChCtrlList pCtrl = GetControlInfo( i );

					if ( pCtrl->pWnd )
					{
						pWnd = pCtrl->pWnd;
						break;
					}
				}
			}
		}

	}
	if ( pWnd )
	{
		pWnd->SetFocus();
	}
}

////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////

/*----------------------------------------------------------------------------
	ChHTMLWnd class  : All form support methods
----------------------------------------------------------------------------*/



ChHTMLFormLst* ChHtmlView::GetFormList()
{
	if ( !m_pformLst )
	{
		m_pformLst = new ChHTMLFormLst;
	}
	return 	m_pformLst;
}

ChHTMLForm * ChHtmlView::GetCurrentForm()
{
	if ( m_pformLst )
	{
		return m_pformLst->GetHead();
	}
	else
	{
		return NULL;
	}
}


void ChHtmlView::CreateControl( ChCtrlList& ctrlObj, chuint32 luAttr,
								bool bbChecked )
{
	ChHTMLForm*		pForm = GetCurrentForm();

	if (!pForm)
	{
	 	return;
	}

	ChRect  	rtPos( 0, 0, 0, 0 );
	ChRect		spaceExtra( 10, 0, 0, 0 );

	#if defined( CH_MSW )
	{
		ctrlObj.pWnd = 0;

		switch ( ctrlObj.iType )
		{
			case VAL_CHECKBOX:
			case VAL_RADIO:
			{
				chuint32	luStyle = WS_CHILD | WS_TABSTOP;
				int			iID = pForm->GetTotalControls();

				if (iID)
				{
					pChCtrlList		pInfo = pForm->GetControlInfo( iID - 1 );

					if (pInfo->iType != ctrlObj.iType)
					{
						luStyle	|= WS_GROUP;
					}
					else if (pInfo->pstrName && ctrlObj.pstrName &&
								lstrcmp( pInfo->pstrName, ctrlObj.pstrName ))
					{
						luStyle |= WS_GROUP;
					}
				}
				else
				{
					luStyle |= WS_GROUP;
				}
					
				if ( pForm->GetCheckBoxWidth() && pForm->GetCheckBoxHeight() )
				{
					rtPos.right = pForm->GetCheckBoxWidth();
					rtPos.bottom = pForm->GetCheckBoxHeight();
				}
				else
				{
					rtPos.right = pForm->GetAvgCharWidth()  * 2;
					rtPos.bottom = pForm->GetCharHeight();
				}

				luStyle |= (ctrlObj.iType == VAL_CHECKBOX) ?
							BS_AUTOCHECKBOX : BS_AUTORADIOBUTTON;

				ChFrmButton*	pButton = new ChFrmButton;

				ASSERT( pButton );
				pButton->Create( 0, luStyle, rtPos, this, iID );
				ctrlObj.pWnd = pButton;

				pButton->SetCheck( bbChecked );
				pButton->SetForm( pForm );

				// Add some extra space to the right of the button
				spaceExtra.right = pForm->GetAvgCharWidth();
				break;
			}

			case VAL_HIDDEN:
			{
				break;
			}

			case VAL_IMAGE:
			{
				//ctrlObject.SetType( ChTxtWnd::objectRect );
				//rtPos.right = 100;
				//rtPos.bottom = 100;
				break;
			}

			case VAL_RESET:
			{
				char*	pstrTitle = ctrlObj.pstrValue ? ctrlObj.pstrValue :
														"Reset";
				if (!ctrlObj.size.cx)
				{
					ctrlObj.size.cx = lstrlen( pstrTitle ) + 4; // columns
				}

				if (!ctrlObj.size.cy)
				{
					ctrlObj.size.cy = 1;	// Height in lines
				}

				rtPos.right = pForm->GetAvgCharWidth() * ctrlObj.size.cx;
				rtPos.bottom = pForm->GetCharHeight() * ctrlObj.size.cy +
								(pForm->GetCharHeight() / 2);

				ChResetBtn*		pButton = new ChResetBtn;

				ASSERT( pButton );
				pButton->Create( pstrTitle,
									WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP,
									rtPos, this, pForm->GetTotalControls() );
				ctrlObj.pWnd = pButton;
				pButton->SetForm( pForm );
				break;
			}

			case VAL_SUBMIT:
			{
				char*	pstrTitle = ctrlObj.pstrValue ? ctrlObj.pstrValue :
														"Submit";
				if (!ctrlObj.size.cx)
				{
					ctrlObj.size.cx = lstrlen( pstrTitle ) + 4; // columns
				}

				if (!ctrlObj.size.cy)
				{
					ctrlObj.size.cy = 1;	// Height in lines
				}

				rtPos.right = pForm->GetAvgCharWidth() * ctrlObj.size.cx;
				rtPos.bottom = pForm->GetCharHeight() * ctrlObj.size.cy +
								(pForm->GetCharHeight() / 2);

				ChSubmitBtn*	pButton = new ChSubmitBtn;

				ASSERT( pButton );
				pButton->Create( pstrTitle,
									WS_CHILD | BS_DEFPUSHBUTTON | WS_TABSTOP,
									rtPos, this, pForm->GetTotalControls() );
				ctrlObj.pWnd = pButton;
				pButton->SetForm( pForm );	// set the form associated with this button
				break;
			}

			case VAL_TEXT:
			case VAL_PASSWORD:
			case TYPE_MULTILINETEXT:
			{
				chuint32	luStyle =  WS_CHILD | WS_BORDER | WS_TABSTOP;

				if (ctrlObj.iType ==  TYPE_MULTILINETEXT)
				{
					luStyle |= ES_MULTILINE | WS_VSCROLL;
					if (!ctrlObj.size.cx)
					{
						ctrlObj.size.cx = 80; // columns
					}
					//ctrlObj.size.cx = ( ctrlObj.size.cx * 70 ) / 100;

					if (!ctrlObj.size.cy)
					{
						ctrlObj.size.cy = 5; // lines
					}
				}
				else
				{
					if (!ctrlObj.size.cx)
					{
						ctrlObj.size.cx = 20; // columns
					}

					ctrlObj.size.cy = 1;
					luStyle |= ES_AUTOHSCROLL;
					if (ctrlObj.iType == VAL_PASSWORD)
					{
						luStyle |= ES_PASSWORD;
					}
				}

				rtPos.right = pForm->GetAvgCharWidth() * ctrlObj.size.cx +
								pForm->GetEditExtraWidth();
				rtPos.bottom = pForm->GetCharHeight() * ctrlObj.size.cy +
								(pForm->GetCharHeight() >> 1);

				ChFrmEtxt*	pEtxt = new ChFrmEtxt;

				ASSERT( pEtxt );

				if ( ctrlObj.iType ==  TYPE_MULTILINETEXT  )
				{
					pEtxt->SetMultiline( true );
				}
				pEtxt->Create( luStyle, rtPos, this,
								pForm->GetTotalControls() );
				ctrlObj.pWnd = pEtxt;
											// Set the default text
				if (ctrlObj.pstrValue)
				{
					pEtxt->SetWindowText( ctrlObj.pstrValue );
				}
											// Enforce text limit
				if (ctrlObj.iLimit)
				{
					pEtxt->LimitText( ctrlObj.iLimit );
				}
											/* Set the current form to handle
												submit if there is no submit
												button */
				pEtxt->SetForm( pForm );
				break;
			}

			case TYPE_POPUPLIST:
			{
				chuint32	luStyle =  WS_CHILD | WS_BORDER | CBS_DROPDOWNLIST |
										WS_TABSTOP | WS_VSCROLL;

				ctrlObj.size.cx = 4;		// Default width in characters

				if (!ctrlObj.size.cy)
				{
					ctrlObj.size.cy = 7;	// Default height in lines
				}

				rtPos.right = (pForm->GetAvgCharWidth() * ctrlObj.size.cx) +
								pForm->GetPopupListExtraWidth();
				rtPos.bottom = pForm->GetCharHeight() * ctrlObj.size.cy;

				ChFrmCombo*		pCombo = new ChFrmCombo;

				ASSERT( pCombo );

				pCombo->Create( luStyle, rtPos, this, pForm->GetTotalControls() );
				ctrlObj.pWnd = pCombo;
											/* The height of the popup list box
												for text view is the height of
												the edit box */
				ChRect		rtHt;

				pCombo->GetWindowRect( rtHt );
				rtPos.bottom = rtHt.Height();
				pCombo->SetForm( pForm );
				break;
			}

			case TYPE_LIST:
			case TYPE_LISTMULTI:
			{
				chuint32	luStyle =  WS_CHILD | WS_BORDER | WS_VSCROLL |
										WS_TABSTOP | WS_VSCROLL;

				if (TYPE_LISTMULTI == ctrlObj.iType)
				{
					luStyle |= LBS_MULTIPLESEL;
				}

				ctrlObj.size.cx = 15;
				if (!ctrlObj.size.cy)
				{
					ctrlObj.size.cy = 7;	// items
				}

				rtPos.right = pForm->GetAvgCharWidth() * ctrlObj.size.cx;
				rtPos.bottom = pForm->GetCharHeight() * ctrlObj.size.cy;

				ChFrmList*	pList = new ChFrmList;

				ASSERT( pList );
				pList->Create( luStyle, rtPos, this,
								pForm->GetTotalControls() );
				ctrlObj.pWnd = pList;
				pList->SetForm( pForm );
				break;
			}
		}

		if ((ctrlObj.iType != VAL_HIDDEN) && ctrlObj.pWnd)
		{
			ChSize		size;

			size.cx = rtPos.right;
			size.cy = rtPos.bottom;

			#if defined( CH_MSW )

			ctrlObj.pWnd->SetFont( pForm->GetControlFont() );

			ChObjControl*	pObjCtl = new ChObjControl( size, spaceExtra,
														luAttr, ctrlObj.pWnd );
			#else

			ChObjControl*	pObjCtl = new ChObjControl( size, spaceExtra,
														luAttr );
			#endif

			ChTxtObject		ctrlObject( pObjCtl );

			ctrlObject.SetStyle( ChTxtWnd::textObject |
									ChTxtWnd::textAlwaysUpdate |
									m_pParser->GetTextStyle()->GetStyle() );
			ctrlObject.SetLeftIndent( m_pParser->GetTextStyle()->GetLeftIndent() );

			ChTxtWnd::AppendObject( &ctrlObject );

			m_pParser->SetLastChar( TEXT( 'C' ) ); // assume control as some printable char

			m_pParser->CommitBuffer();
		}
	}
	#else	// defined( CH_MSW )
	{
		cerr << "XXX Not processing form element: " << __FILE__ << ":"
						<< __LINE__ << endl;
	}
	#endif	// defined( CH_MSW )

	pForm->AddControl( ctrlObj );
											/* Save the current index for list
												and combo box on the stack */

	if ((TYPE_LISTMULTI == ctrlObj.iType) || (TYPE_LIST == ctrlObj.iType) ||
		(TYPE_POPUPLIST == ctrlObj.iType ) || ( TYPE_MULTILINETEXT == TYPE_MULTILINETEXT) )
	{
		ChStackData*	pdata;

		pdata = m_pParser->HTMLStack().Peek( m_pParser->HTMLStack().GetTopIndex() );
		if (pdata->iType == HTML_SELECT || pdata->iType == HTML_TEXTAREA )
		{
			pdata->luModifier = pForm->GetTotalControls() - 1;
		}
	}
}


void ChHtmlView::ClearForms( )
{
	// free all the forms
	if ( m_pformLst && m_pformLst->GetCount())
	{

		ChTxtWnd::ClearForms( );

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
}

#ifdef CH_MSW

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA
#endif

/////////////////////////////////////////////////////////////////////////////
// ChResetBtn

ChResetBtn::ChResetBtn()
{
}

ChResetBtn::~ChResetBtn()
{
}


BEGIN_MESSAGE_MAP(ChResetBtn, CButton)
	//{{AFX_MSG_MAP(ChResetBtn)
	ON_WM_LBUTTONUP()
	ON_WM_KEYUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// ChResetBtn message handlers
/////////////////////////////////////////////////////////////////////////////

void ChResetBtn::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if ( ChildWindowFromPoint( point ) )
	{
		GetForm()->ResetForm();
	}
	CButton::OnLButtonUp(nFlags, point);
}

void ChResetBtn::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	if ( nChar == VK_TAB )
	{

		GetForm()->SetNextTabFocus( this, (GetKeyState( VK_SHIFT ) & 0x8000) != FALSE );
	}

	CButton::OnKeyUp(nChar, nRepCnt, nFlags);
}


// ChSubmitBtn

ChSubmitBtn::ChSubmitBtn() : m_pForm( 0 )
{
}

ChSubmitBtn::~ChSubmitBtn()
{
}


BEGIN_MESSAGE_MAP(ChSubmitBtn, CButton)
	//{{AFX_MSG_MAP(ChSubmitBtn)
	ON_WM_LBUTTONUP()
	ON_WM_KEYUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// ChSubmitBtn message handlers
/////////////////////////////////////////////////////////////////////////////
void ChSubmitBtn::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CButton::OnLButtonUp(nFlags, point);

	if ( ChildWindowFromPoint( point ) )
	{
		// for named submit buttons we need to know which button was selected 
		// to send the name/value pair
		for( int i = 0; i < GetForm()->GetTotalControls(); i++ )
		{
			pChCtrlList pCtrl = GetForm()->GetControlInfo( i );

			if ( pCtrl->pWnd == this )
			{
				pCtrl->boolDefState = true;
				break;
			}
		}

		GetForm()->SubmitForm();
	}

}




void ChSubmitBtn::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	if ( nChar == VK_TAB )
	{

		GetForm()->SetNextTabFocus( this, (GetKeyState( VK_SHIFT ) & 0x8000) != FALSE );
	}
	CButton::OnKeyUp(nChar, nRepCnt, nFlags);
}

/////////////////////////////////////////////////////////////////////////////
// ChFrmCombo

ChFrmCombo::ChFrmCombo()
{
}

ChFrmCombo::~ChFrmCombo()
{
}


BEGIN_MESSAGE_MAP(ChFrmCombo, CComboBox)
	//{{AFX_MSG_MAP(ChFrmCombo)
	ON_WM_KEYUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// ChFrmCombo message handlers
/////////////////////////////////////////////////////////////////////////////

void ChFrmCombo::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	if ( nChar == VK_TAB )
	{

		GetForm()->SetNextTabFocus( this, (GetKeyState( VK_SHIFT ) & 0x8000) != FALSE );
	}

	CComboBox::OnKeyUp(nChar, nRepCnt, nFlags);
}




// ChFrmButton

ChFrmButton::ChFrmButton()
{
}

ChFrmButton::~ChFrmButton()
{
}


BEGIN_MESSAGE_MAP(ChFrmButton, CButton)
	//{{AFX_MSG_MAP(ChFrmButton)
	ON_WM_KEYUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// ChFrmButton message handlers
/////////////////////////////////////////////////////////////////////////////

void ChFrmButton::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	if ( nChar == VK_TAB )
	{

		GetForm()->SetNextTabFocus( this, (GetKeyState( VK_SHIFT ) & 0x8000) != FALSE );
	}
	CButton::OnKeyUp(nChar, nRepCnt, nFlags);
}



// ChFrmEtxt

ChFrmEtxt::ChFrmEtxt() :m_pForm(0), m_boolMultiline( false )
{
}

ChFrmEtxt::~ChFrmEtxt()
{
}


BEGIN_MESSAGE_MAP(ChFrmEtxt, CEdit)
	//{{AFX_MSG_MAP(ChFrmEtxt)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// ChFrmEtxt message handlers
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void ChFrmEtxt::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{

	// TODO: Add your message handler code here and/or call default
	if ( nChar == VK_TAB && !m_boolMultiline )
	{
		MSG msg;
		::PeekMessage( &msg, GetSafeHwnd(), WM_CHAR, WM_CHAR, PM_REMOVE );
		return;
	}
	else if ( nChar == VK_RETURN && !m_boolMultiline )
	{

		if ( GetForm()->GetTotalControls() == 1 )
		{
		 	GetForm()->SubmitForm();
		}
		else if ( GetForm()->GetTotalControls() == 2 )
		{
			for ( int i = 0; i < GetForm()->GetTotalControls(); i++ )
			{
				pChCtrlList pCtrl = GetForm()->GetControlInfo( i );

				if ( pCtrl->pWnd == this && pCtrl->iType == VAL_TEXT )
				{
				 	GetForm()->SubmitForm();
					break;
				}
			}
		}
	}
	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

void ChFrmEtxt::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default


	if ( nChar == VK_TAB )
	{
		if ( !m_boolMultiline )
		{
			GetForm()->SetNextTabFocus( this,  (GetKeyState( VK_SHIFT ) & 0x8000) != FALSE );
			return;
		}
	}
	else if ( nChar == TEXT( 'C' ) )
	{
		if ( GetKeyState( VK_CONTROL ) & 0x8000  )
		{
			Copy();
		}

	}
	CEdit::OnKeyUp(nChar, nRepCnt, nFlags);
}


// ChFrmList

ChFrmList::ChFrmList()
{
}

ChFrmList::~ChFrmList()
{
}


BEGIN_MESSAGE_MAP(ChFrmList, CListBox)
	//{{AFX_MSG_MAP(ChFrmList)
	ON_WM_KEYUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// ChFrmList message handlers

void ChFrmList::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	if ( nChar == VK_TAB )
	{

		GetForm()->SetNextTabFocus( this, (GetKeyState( VK_SHIFT ) & 0x8000) != FALSE );
	}

	CListBox::OnKeyUp(nChar, nRepCnt, nFlags);
}




#endif // CH_MSW

// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
