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
#include "ChHtmlView.h"
#include "ChHtmSym.h"
#include "ChHtmlParser.h"
#include "ChFormTags.h"

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	ChHTMLForm class
----------------------------------------------------------------------------*/

#define SWP_SETSIZE		(SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER)



///////////////////////////////////////////////////////////////////////////////
/////////////
////////////   ChFormTag implementation 
////////////
///////////////////////////////////////////////////////////////////////////////


ChFormTag::ChFormTag(  ChHtmlParser *pParser ) : ChBodyElements(  pParser )
{
	m_iTokenID = HTML_FORM;
	m_luAttrs = attrAddLineAbove | attrAddLineBelow |
						 attrHasArguments | attrCallProcessTag;
}

void ChFormTag::ProcessArguments( pChArgList pList, int iArgCount )
{
	ChHTMLForm*		pForm = new ChHTMLForm( GetHtmlView() );

	ASSERT( pForm );

	for (int i = 0; i < iArgCount; i++)
	{
		switch (pList[i].iArgType)
		{
			case ARG_ACTION:
			{
				pForm->SetAction( (const char*) pList[i].argVal );
				break;
			}
			case ARG_MD5 :
			{
				pForm->SetMD5( (const char*)pList[i].argVal );
				break;
			}
			case ARG_METHOD:
			{
				switch( pList[i].argVal )
				{
					case VAL_POST:
					{
						pForm->SetMethod( ChHTMLForm::methodPost );
						break;
					}
					case VAL_GET:
					{
						pForm->SetMethod( ChHTMLForm::methodGet );
						break;
					}
					default:
					{
						pForm->SetMethod( ChHTMLForm::methodXCHCMD );
						break;
					}
				}
				break;
			}
		}
	}


	GetHtmlView()->GetFormList()->AddHead( pForm );
} 

chint32 ChFormTag::ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount )
{
	if ( lStart >= lCount )
	{
		return lStart;
	}


	// skip all white space after the tag
	while ( lStart < lCount && 	IS_WHITE_SPACE( pstrBuffer[lStart] ) )
	{
		lStart++;
	}

	if ( pstrBuffer[lStart] != TEXT( '<' ) )
	{
		lStart = ChBodyElements::ProcessTag( pstrBuffer, lStart, lCount );
	}
	return lStart;
}

///////////////////////////////////////////////////////////////////////////////
/////////////
////////////   ChFormInputTag implementation 
////////////
///////////////////////////////////////////////////////////////////////////////

ChFormInputTag::ChFormInputTag(  ChHtmlParser *pParser ) : 	ChBodyElements(  pParser )
{
	m_iTokenID = HTML_INPUT;
	m_luAttrs |= attrHasArguments | attrCallProcessTag;
}

void ChFormInputTag::ProcessArguments( pChArgList pList, int iArgCount )
{
	ChCtrlList		ctrlObj;
	chuint32		luAlign = ChTxtWnd::objAttrMiddle;
	bool			bbChecked = false;

	ChMemClearStruct( &ctrlObj );
	ctrlObj.iType = VAL_TEXT;
	ctrlObj.boolDefSize = true;
									// process all the attributes

	for (int i = 0; i < iArgCount; i++)
	{
		switch ( pList[i].iArgType )
		{
			case ARG_ALIGN:
			{						// map alignment to to object attrs

				luAlign = 0;
				switch( pList[i].argVal )
				{
					case VAL_CENTER :
					case VAL_ABSMIDDLE :
					{
						luAlign = 	ChTxtWnd::textCenter;
						break;
					}
					case VAL_RIGHT :
					{
						luAlign = 	ChTxtWnd::textRight;
						break;
					}
					case VAL_LEFT :
					{
						luAlign = 	ChTxtWnd::textLeft;
						break;
					}
					default :
					{
						break;
					}

				}
				break;
			}

			case ARG_CHECKED:
			{
				ctrlObj.boolDefState = true;
				bbChecked = true;
				break;
			}

			case ARG_MAXLENGTH:
			{
				ctrlObj.iLimit= (int)pList[i].argVal;
				break;
			}

			case ARG_NAME:
			{
				ctrlObj.pstrName = (char*)pList[i].argVal;
				break;
			}

			case ARG_VALUE:
			{
				ctrlObj.pstrValue = (char*)pList[i].argVal;
				break;
			}

			case ARG_SIZE:
			{
				ctrlObj.size.cx = (int)pList[i].argVal;
				ctrlObj.boolDefSize = false;
				break;
			}

			case ARG_TYPE:
			{
				ctrlObj.iType = (int)pList[i].argVal;
				break;
			}
		}
	}
									// create the control

 	GetHtmlView()->CreateControl( ctrlObj, luAlign, bbChecked );
} 

chint32 ChFormInputTag::ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount )
{
	if ( lStart >= lCount )
	{
		return lStart;
	}


	// skip all white space after the tag
	if( !(m_pParser->GetTextStyle()->GetStyle() & ChTxtWnd::textPreFormat) )
	{
		while ( lStart < lCount && 	IS_WHITE_SPACE( pstrBuffer[lStart] ) )
		{
			lStart++;
		}
	}

	if ( pstrBuffer[lStart] != TEXT( '<' ) )
	{
		lStart = ChBodyElements::ProcessTag( pstrBuffer, lStart, lCount );
	}
	return lStart;
}


///////////////////////////////////////////////////////////////////////////////
/////////////
////////////   ChFormSelectTag implementation 
////////////
///////////////////////////////////////////////////////////////////////////////

ChFormSelectTag::ChFormSelectTag(  ChHtmlParser *pParser ) :ChBodyElements(  pParser )
{
	m_iTokenID = HTML_SELECT;
	m_luAttrs = attrSaveState | attrHasArguments | attrCallProcessTag | attrTrimRight;
}

void ChFormSelectTag::ProcessArguments( pChArgList pList, int iArgCount )
{
	ChCtrlList		ctrlObj;

	ChMemClearStruct( &ctrlObj );
	ctrlObj.iType = TYPE_POPUPLIST;
	ctrlObj.boolDefSize = true;
									// Process all the attributes
	for (int i = 0; i < iArgCount; i++)
	{
		switch ( pList[i].iArgType )
		{
			case ARG_MULTIPLE:
			{
				ctrlObj.iType = TYPE_LISTMULTI;
				break;
			}

			case ARG_SIZE:
			{
				ctrlObj.size.cy = (int)pList[i].argVal;
				ctrlObj.iType = TYPE_LIST;
				break;
			}

			case ARG_NAME:
			{
				ctrlObj.pstrName = (char*)pList[i].argVal;
				break;
			}
			case ARG_VALUE:
			{
				ctrlObj.pstrValue = (char*)pList[i].argVal;
				break;
			}
			default :
			{
				break;
			}
		}
	}
									// Create the control
 	GetHtmlView()->CreateControl( ctrlObj );
} 

chint32 ChFormSelectTag::ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount )
{

	if ( lStart >= lCount )
	{
		return lStart;
	}


	// skip all white space after the tag
	while ( lStart < lCount && 	IS_WHITE_SPACE( pstrBuffer[lStart] ) )
	{
		lStart++;
	}

	if ( pstrBuffer[lStart] != TEXT( '<' ) )
	{
		lStart = ChBodyElements::ProcessTag( pstrBuffer, lStart, lCount );
	}
	return lStart;
}


///////////////////////////////////////////////////////////////////////////////
/////////////
////////////   ChFormTextAreaTag implementation 
////////////
///////////////////////////////////////////////////////////////////////////////

ChFormTextAreaTag::ChFormTextAreaTag(  ChHtmlParser *pParser ) : ChHtmlTag(  pParser )
{
	m_iTokenID = HTML_TEXTAREA;
	m_luAttrs = attrSaveState | attrHasArguments | attrCallProcessTag | attrTrimRight;
}

void ChFormTextAreaTag::ProcessArguments( pChArgList pList, int iArgCount )
{
	ChCtrlList		ctrlObj;

	ChMemClearStruct( &ctrlObj );
	ctrlObj.iType = TYPE_MULTILINETEXT;
	ctrlObj.boolDefSize = true;
									// process all the attributes
	for( int i = 0; i < iArgCount; i++ )
	{
		switch ( pList[i].iArgType )
		{
			case ARG_NAME:
			{
				ctrlObj.pstrName = (char*)pList[i].argVal;
				break;
			}
			case ARG_ROWS:
			{
				ctrlObj.size.cy = (int)pList[i].argVal;
				break;
			}
			case ARG_COLS:
			{
				ctrlObj.size.cx = (int)pList[i].argVal;
				break;
			}
			case ARG_MAXLENGTH:
			{
				ctrlObj.iLimit= (int)pList[i].argVal;
				break;
			}
			case ARG_VALUE:
			{
				ctrlObj.pstrValue = (char*)pList[i].argVal;
				break;
			}
			default:
				break;
		}
	}
									// create the control
 	GetHtmlView()->CreateControl( ctrlObj );
} 

chint32 ChFormTextAreaTag::ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount )
{

	if ( lStart >= lCount )
	{
		return lStart;
	}


	// skip all white space after the tag
	while ( lStart < lCount && 	IS_WHITE_SPACE( pstrBuffer[lStart] ) )
	{
		lStart++;
	}

	while (lStart < lCount)
	{
		if (pstrBuffer[lStart] != TEXT( '<' ))
		{
			char	strChar = pstrBuffer[lStart++];
			char 	strPrev = 0;

			switch( strChar )
			{
				case  TEXT( '\r' ) :
				{
					m_pParser->AppendChar( TEXT( '\r' ) );
					m_pParser->AppendChar( TEXT( '\n' ) );
					strPrev = strChar;
					break;
				}
				case  TEXT( '\n' ) :
				{
					if ( strPrev  != TEXT( '\r' ) )
					{
						m_pParser->AppendChar( TEXT( '\r' ) );
						m_pParser->AppendChar( TEXT( '\n' ) );
					}
					strPrev = strChar;
					break;
				}
				case TEXT( '&' ) :
				{
					if ( m_pParser->MapEntityToChar( pstrBuffer, 
													--lStart, lCount, strChar ) )
					{
						if ( strChar == TEXT( '\n' ) )
						{
							m_pParser->AppendChar( TEXT( '\r' ) );
							m_pParser->AppendChar( TEXT( '\n' ) );
						}
						else
						{
							m_pParser->AppendChar( strChar  );
						}
					}
					else
					{
						return lStart;
					}
					break;
				}
				default :
				{
					strPrev = strChar;
					m_pParser->AppendChar( strChar );
				}
			}
		}
		else
		{
			
			bool	boolEnd;
			int iToken = m_pParser->LookUpTag( pstrBuffer, lStart, lCount, boolEnd );
			if ( m_iTokenID == iToken  && boolEnd )
			{ 
				break;
			}
			else
			{
				m_pParser->AppendChar( pstrBuffer[lStart++]  );
			}
		}
	}

	if (m_pParser->GetBufferIndex())
	{
		int		i = m_pParser->GetBufferIndex() - 1;

		while((i >= 0) && (m_pParser->GetBuffer()[ i ] == TEXT( ' ' )))
		{
			i--;
		}
		m_pParser->GetBuffer()[ i + 1 ] = 0;

		ChHTMLForm*		pForm = GetHtmlView()->GetCurrentForm();
		int				iIndex = m_pParser->HTMLStack().GetTopIndex();
		ChStackData*	pStack;
		pStack = m_pParser->HTMLStack().Peek( iIndex );

		if (pForm)
		{
			pChCtrlList		pInfo;

			pInfo = pForm->GetControlInfo( (int)pStack->luModifier );

			#if defined( CH_MSW )
			{
				if (pInfo->iType == TYPE_MULTILINETEXT )
				{
					ChFrmEtxt*		pEtxt = (ChFrmEtxt*)pInfo->pWnd;

					pEtxt->SetWindowText( m_pParser->GetBuffer() );
				}
			}
			#endif
		}
	}
	m_pParser->SetBufferIndex( 0 );

	return lStart;
}

///////////////////////////////////////////////////////////////////////////////
/////////////
////////////   ChFormOptionTag implementation 
////////////
///////////////////////////////////////////////////////////////////////////////

ChFormOptionTag::ChFormOptionTag(  ChHtmlParser *pParser ) : 
						ChHtmlTag(  pParser )
{
	m_iTokenID = HTML_OPTION;
	m_luAttrs =  attrHasArguments | attrCallProcessTag | attrTrimRight;
}

void ChFormOptionTag::ProcessArguments( pChArgList pList, int iArgCount )
{
	ChCtrlList		ctrlObj;

	ChMemClearStruct( &ctrlObj );
	ctrlObj.iType = TYPE_LIST_ELEMENT;
	ctrlObj.boolDefSize = true;
									// process all the attributes

	for( int i = 0; i < iArgCount; i++ )
	{
		switch ( pList[i].iArgType )
		{
			case ARG_SELECTED:
			{
				// select the current element
				ctrlObj.iType = TYPE_LIST_ELEMENT_SEL;
				break;
			}
			case ARG_VALUE:
			{
				ctrlObj.pstrValue = (char*)pList[i].argVal;
				break;
			}
		}
	}
	ChHTMLForm *pForm = GetHtmlView()->GetCurrentForm();
	if ( pForm )
	{
		pForm->AddControl( ctrlObj );
	}
} 

chint32 ChFormOptionTag::ProcessTag( const char* pstrBuffer, chint32 lStart, chint32 lCount )
{


	if (lStart >= lCount)
	{
		return lStart;
	}


	int				iIndex = m_pParser->HTMLStack().GetTopIndex();
	ChStackData*	pStack;

	pStack = m_pParser->HTMLStack().Peek( iIndex );

	if (pStack->iType != HTML_SELECT)
	{										/* Top of the stack is not select,
												ignore all data for option */

		while ((lStart < lCount) && (pstrBuffer[lStart] != TEXT( '<' )))
		{
			lStart++;
		}
		return lStart;
	}
	else
	{
		while ((lStart < lCount) && IS_WHITE_SPACE( pstrBuffer[lStart] ))
		{
			lStart++;
		}

		while (lStart < lCount)
		{
			if (pstrBuffer[lStart] != TEXT( '<' ))
			{
				char	strChar = pstrBuffer[lStart++];

				if (!IS_WHITE_SPACE( strChar ))
				{
					if ( strChar == TEXT( '&' ) )
					{
						if ( m_pParser->MapEntityToChar( pstrBuffer, 
														--lStart, lCount, strChar ) )
						{
							m_pParser->AppendChar( strChar  );
						}
					}
					else
					{
						m_pParser->AppendChar( strChar  );
					}
				}
				else //if (m_pParser->GetLastChar() != TEXT( ' ' ))
				{
					if ( !IS_WHITE_SPACE( m_pParser->GetLastChar() ) )
					{

						m_pParser->AppendChar( TEXT( ' ' ) );
					}

											/* Skip till the next non-white
												space */

					while ((lStart < lCount) &&
							IS_WHITE_SPACE( pstrBuffer[lStart] ))
					{
						lStart++;
					}
				}
			}
			else
			{
				break;
			}
		}
	}

	if (m_pParser->GetBufferIndex())
	{
		int		i = m_pParser->GetBufferIndex() - 1;

		while((i >= 0) && (m_pParser->GetBuffer()[ i ] == TEXT( ' ' )))
		{
			i--;
		}
		m_pParser->GetBuffer()[ i + 1 ] = 0;
	}
	else
	{
		m_pParser->GetBuffer()[0] = 0;
	}

	ChHTMLForm*		pForm = GetHtmlView()->GetCurrentForm();

	if (pForm)
	{
		pChCtrlList		pInfo;

		pInfo = pForm->GetControlInfo( (int)pStack->luModifier );

		#if defined( CH_MSW )
		{
			if (pInfo->iType == TYPE_POPUPLIST)
			{
				ChFrmCombo*		pCombo = (ChFrmCombo*)pInfo->pWnd;
				int				iIndex;
				int				iLastControl;
				pChCtrlList		pItemInfo;
				ChString			strText( m_pParser->GetBuffer() );

				iIndex = pCombo->AddString( strText );
				iLastControl = (int)pForm->GetTotalControls() - 1;
				pItemInfo = pForm->GetControlInfo( iLastControl );

				if (pInfo->boolDefSize)
				{						/* See if we need to resize this
											control to the contents */
					ChRect	rtOldSize;
					int		iNewWidth;
					CDC*	pDC = pInfo->pWnd->GetDC();
					CSize	textSize;

					pInfo->pWnd->GetWindowRect( rtOldSize );

					textSize = pDC->GetTextExtent( strText,
													strText.GetLength() );
					iNewWidth = textSize.cx +
								pForm->GetPopupListExtraWidth() + pForm->GetAvgCharWidth();
					pInfo->pWnd->ReleaseDC( pDC );

					int 	iNumElements = pCombo->GetCount( ) + 1;

					int		iHeight = pCombo->GetItemHeight( 0 ) *
										(iNumElements <  pInfo->size.cy ?
										iNumElements : pInfo->size.cy);

					iHeight += rtOldSize.Height();


					if ( iNewWidth < rtOldSize.Width() )
					{
						iNewWidth = rtOldSize.Width();
					}

					pInfo->pWnd->SetWindowPos( 0, 0, 0, iNewWidth,
													iHeight, SWP_SETSIZE );
				}

				if ((iIndex != CB_ERR) && (pItemInfo->iType == TYPE_LIST_ELEMENT_SEL))
				{
					pCombo->SetCurSel( iIndex );
				}
				else if ( CB_ERR == pCombo->GetCurSel( ))
				{
					pCombo->SetCurSel( 0 );
				}
			}
			else
			{
				ChFrmList*		pList = (ChFrmList*)pInfo->pWnd;
				int				iLastControl;
				pChCtrlList		pItemInfo;
				ChString			strText( m_pParser->GetBuffer() );

				iIndex = pList->AddString( strText );
				iLastControl = (int)pForm->GetTotalControls() - 1;
				pItemInfo = pForm->GetControlInfo( iLastControl );

				if (pInfo->boolDefSize)
				{						/* See if we need to resize this
											control to the contents */
					ChRect	rtOldSize;
					int		iNewWidth;
					CDC*	pDC = pInfo->pWnd->GetDC();
					CSize	textSize;

					pInfo->pWnd->GetWindowRect( rtOldSize );

					textSize = pDC->GetTextExtent( strText,
													strText.GetLength() );
					iNewWidth = textSize.cx +
								pForm->GetListExtraWidth() + pForm->GetAvgCharWidth();
					pInfo->pWnd->ReleaseDC( pDC );

					int 	iNumElements = pList->GetCount();
					int		iHeight = pList->GetItemHeight( 0 ) *
										(iNumElements <  pInfo->size.cy ?
										iNumElements : pInfo->size.cy);

					CRect rtWnd, rtClient;

					pList->GetWindowRect( &rtWnd );
					pList->GetClientRect( &rtClient );

					iHeight += ( rtWnd.Height() - rtClient.Height() );

					if ( iNumElements <  pInfo->size.cy )
					{
					 	iNewWidth -= GetSystemMetrics( SM_CXHTHUMB );
					}
					if ( iHeight < rtOldSize.Height() )
					{
						iHeight = rtOldSize.Height();
					}

					if ( iNewWidth < rtOldSize.Width() )
					{
						iNewWidth = rtOldSize.Width();
					}

					{
						pInfo->pWnd->SetWindowPos( 0, 0, 0, iNewWidth,
													iHeight,
													SWP_SETSIZE );
					}
				}

				if ((iIndex != LB_ERR) &&
					(pItemInfo->iType == TYPE_LIST_ELEMENT_SEL))
				{
					if (pInfo->iType == TYPE_LISTMULTI)
					{
						pList->SetSel( iIndex );
					}
					else
					{
						pList->SetCurSel( iIndex );
					}
				}
				else if ( iIndex != LB_ERR && pInfo->iType != TYPE_LISTMULTI )
				{
					if ( LB_ERR == pList->GetCurSel( ))
					{
						pList->SetCurSel( 0 );
					}
				}
			}
		}
		#endif	// defined( CH_MSW )
											/* Reset the buffer index of the
												style we modified */
		m_pParser->SetBufferIndex( 0 );
	}

	return lStart;
}

// $Log$
