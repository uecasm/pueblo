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

	This file consists of the implementation of the ChAccountInfo class.

----------------------------------------------------------------------------*/

#ifndef __CHTLBAR_H
#define __CHTLBAR_H

#include <afxwin.h>		
#include <afxext.h>	 

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif

 
/*
 To get the tooltip and fly-by help for tool bars when using replace the CToolBar class
with ChToolBar and specify  CBRS_TOOLTIPS  and CBRS_FLYBY flags to enable the feature.
*/

#if defined( WIN32 ) || defined( CH_UNIX )        
typedef	 CToolBar			ChToolBar;	// We get it for free in MFC 3.0

#else    

// These constants are defined in MFC 3.0 to provide the tool tip and fly-by
// help for tool bars.

#define CBRS_TOOLTIPS       0x0010L
#define CBRS_FLYBY          0x0020L

#define CBRS_BORDER_LEFT    0x0100L
#define CBRS_BORDER_TOP     0x0200L
#define CBRS_BORDER_RIGHT   0x0400L
#define CBRS_BORDER_BOTTOM  0x0800L
#define CBRS_BORDER_ANY     0x0F00L
	
 
 /* 

 Tooltip class provides a popup window which is used to display the 
 the tool tip text. This is dynamically created by the ChToolBar class
 which manages the creation display and cleaning up of the object.

*/
 
class CToolTip : public CWnd
{
	friend class ChToolBar;

	private:
		CToolTip(CWnd* pParentWnd);		
	    ~CToolTip();
	   
	    void ShowToolTip(UINT nID );// Show a tool tip window
	    void HideToolTip();			// Hide the current tip.
		CString 	m_strTipText;	// Text to be displayed when tooltip  is shown
		CFont*		m_pFont;		// Font used to display tool tip text
		UINT		m_uShowToolTip;	// ID of the item being shown, 0 for none
		UINT 		m_uClearance;	// Number of pixels to leave between text and edges of tooltip window 
		UINT		m_uCursorHt;	// Height of the cursor
		UINT		m_uCursorWd;	// Width of cursor
		HBRUSH		m_hBrush;		// Brush for window background 
	
	protected:
		// Generated message map functions
		//{{AFX_MSG(CToolTip)
		afx_msg int 	OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void 	OnPaint();
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
};
/* 
	The ChToolBar class replaces the CToolBar class under MFC 2.5 to provide the tool 
	tip and fly-by help message. ChToolBar is derived from CToolBar class.
	The text for the window is loaded from the string table whose ID is the ID 
	of the toolbar button. MFC 3.0 uses the text loaded till the first new-line 
 	character for the status bar help and the rest for the tool tip, the current 
 	implementation  of ChToolBar uses the same logic to be consistant with MFC 3.0 
 */

	
class ChToolBar : public CToolBar
{
	DECLARE_DYNCREATE(ChToolBar)
	
	
	public:
		ChToolBar();						//  constructor
		~ChToolBar();						//  destructor
	    
	    // Override default CToolBar::Create() to initialize this class.
		bool Create(CWnd* pParentWnd, 
				DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_TOP,	
				UINT nID = AFX_IDW_TOOLBAR);
		DWORD GetBarStyle()		{ return m_dwStyle; }
		void  SetBarStyle( DWORD dwStyle ) {  m_bEnableToolTips = (bool)(dwStyle & CBRS_TOOLTIPS );
											  m_bEnableFlyBy =  (bool)(dwStyle & CBRS_FLYBY ); }
	protected :
		ChSize CalcFixedLayout(bool bStretch, bool bHorz);
		void  CalcInsideRect(RECT& rect) const;
		inline chint32 GetLeftBorder() const 
					{  
						return m_uBorderWidth; 
					} 
		inline chint32 GetRightBorder() const 
					{ 
						return m_uBorderWidth;    
					}
		inline chint32 GetTopBorder() const 
					{ 
						return m_uBorderWidth;    
					}
		inline chint32 GetBottomBorder() const 
					{ 
						return m_uBorderWidth;    
					}

	    DWORD			m_dwStyle;
		
	private:
		UINT			GetToolBarBtnUnderMouse();
		UINT			m_uShowTip;			// ID of the toolbar button
		CToolTip*		m_pToolTipWnd;		// Pointer to the tooltip window
	    UINT			m_uWait;			// Number of milliseconds to wait before showing tip (After mouse has stopped)
	    CPoint			m_ptCurPos;			// Cursor position when we decided to show a tool tip
	    bool			m_bMouseDown;		// Is the mouse left button down?
	    bool			m_bEnableFlyBy;		// Should WM_SETMESSAGETEXT messages be sent for toolbar items?
	    bool			m_bEnableToolTips;	// Should ToolTips be used?   
		UINT			m_uBorderWidth;		// width of thisn border	    
	protected:
		// Generated message map functions
		//{{AFX_MSG(ChToolBar)
		afx_msg void 	ChToolBar::OnTimer(UINT nIDEvent);
		afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
		afx_msg void 	ChToolBar::OnMouseMove(UINT nFlags, CPoint point);
		afx_msg void	OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void 	OnLButtonUp(UINT nFlags, CPoint point);
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
};

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR    
#endif

#endif  // WIN32
#endif	// __CHTLBAR_H
