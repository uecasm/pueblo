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

	This file consists of interface for the ChSplitter class.

----------------------------------------------------------------------------*/

#if !defined( _CHSPLIT_H )
#define _CHSPLIT_H

#include <ChReg.h>
#include <ChList.h>
#include <ChRect.h>

#ifdef CH_UNIX
#define AFX_IDW_PANE_FIRST 0
#endif

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif

#if defined( CH_MSW )

#ifndef __AFXEXT_H__
	#include <afxext.h>
#endif

											/* Disable warnings about non-
												exported classes for MSW */
	#pragma warning( disable: 4275 )
	#pragma warning( disable: 4251 )

#endif	// defined( CH_MSW )


typedef struct
{
	int		iMinWidth;
	int		iIdealWidth;
	int		iMinHeight;
	int		iIdealHeight;
	int		iCurSize;

} ChPaneInfo;


class ChPane;


/*----------------------------------------------------------------------------
	ChSplitterBanner class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChSplitterBanner : public ChWnd
{
	friend class ChSplitter;				// This class creates the object

	public:
		virtual ~ChSplitterBanner();

		inline ChSplitter* GetSplitter() { return m_pSplitter; }
		inline bool IsTop() const { return m_boolTop; }
		inline const CRect& GetBorders() const { return m_rtBorders; }
		inline chint16 GetEdgeHeight() const { return m_sEdgeHeight; }
		inline CWnd* GetChild() { return m_pChild; }
		inline bool IsHidden() const { return m_boolHidden; }
		inline bool IsWindows95() const { return m_boolWindows95; }

		inline void SetBorders( CRect& rtBorders ) { m_rtBorders = rtBorders; }

		void GetSize( CSize& size ) const;
		void GetDefaultBorders( CRect& rtBorders ) const;

		void SetChild( ChWnd* pChild );
		void SetChildSize( chint16 sWidth, chint16 sHeight );

	public:
											/* ClassWizard generated virtual
												function overrides */
		//{{AFX_VIRTUAL(ChSplitterBanner)
		public:
		virtual BOOL PreTranslateMessage(MSG* pMsg);
		//}}AFX_VIRTUAL

	protected:
		ChSplitterBanner( ChSplitter* pParent, ChPane* pPane,
							const ChString& strName, bool boolTop,
							chint16 sClientWidth, chint16 sClientHeight );

		inline ChPane* GetPane() { return m_pPane; }

		void SizeChild( chint32 lWidth, chint32 lHeight );

	protected:
											// Generated message map functions
		//{{AFX_MSG(ChSplitterBanner)
		afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
		afx_msg void OnSize(UINT nType, int cx, int cy);
		afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
		afx_msg void OnNcPaint();
		afx_msg void OnDestroy();
		afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
		//}}AFX_MSG

		afx_msg LONG ChSplitterBanner::OnCtlColorStatic( WPARAM wParam,
															LPARAM lParam );
		afx_msg LONG OnSplitterBannerRecalc( UINT wParam, LONG lParam );


		DECLARE_MESSAGE_MAP()

	protected:
		static ChString	m_strClass;

		ChPane*			m_pPane;
		ChSplitter*		m_pSplitter;
		bool			m_boolTop;
		chint16			m_sClientWidth;
		chint16			m_sClientHeight;
		chint16			m_sEdgeHeight;
		CRect			m_rtBorders;
		ChWnd*			m_pChild;
		bool			m_boolWindows95;
		bool			m_boolDrawEdgeAvailable;
		bool			m_boolHidden;

		HBRUSH			m_hbrBackColor;
};


/*----------------------------------------------------------------------------
	ChSplitter list destruct helpers
----------------------------------------------------------------------------*/

static void ChDestructHelper( ChSplitterBanner* pItem )
{
	delete pItem;
}


/*----------------------------------------------------------------------------
	ChSplitter class
----------------------------------------------------------------------------*/

#if defined( CH_MSW )
class CH_EXPORT_CLASS ChSplitter : public CWnd
#else	// defined( CH_MSW )
class CH_EXPORT_CLASS ChSplitter
#endif	// defined( CH_MSW )
{
	#if defined( CH_MSW )

	DECLARE_DYNAMIC( ChSplitter )

	#endif	// defined( CH_MSW )

	friend class ChMainFrame;				// This class creates the object
	friend class ChPane;					// Access through this class

	public:

		virtual void RecalcLayout( bool boolHardReset = true );

		ChSplitterBanner* CreateBanner( ChPane* pPane, const ChString& strName,
										bool boolTop, bool boolFrame,
										chint16 sClientWidth,
										chint16 sClientHeight );
		void DestroyBanner( ChWnd* pDestructoBanner );
		ChSplitterBanner* FindBanner( const ChString& strName );

		inline chint16 GetPaneCount() const { return m_sPaneCount; }
		void SwapPanes();
		void TogglePaneOrientation();
	protected:
		enum tagConstants { maxPanes = 2 };

	protected:
		ChSplitter();
		virtual ~ChSplitter();

		BOOL ChSplitter::Create( ChWnd* pParentWnd );

		inline bool IsWindows95() { return m_boolWindows95; }

		#if defined( CH_MSW )

		inline CWnd* GetPane( int iPane ) const
						{
							ASSERT( iPane >= 0 && iPane < maxPanes );

							return m_pPanes[iPane];
						}

		#endif	// defined( CH_MSW )

		inline bool IsEmpty() const { return 0 == m_sPaneCount; }
		inline bool IsSinglePane() const { return 1 == m_sPaneCount; }
		inline bool IsVertical() const { return m_boolVertical; }
		inline bool IsPaneVisible( int iPane ) const
						{
							ASSERT( iPane < maxPanes );
							ASSERT( iPane >= 0 );

							return IsPaneInUse( iPane ) &&
									m_boolPaneShown[iPane];
						}

		void SetWnd( int iPane, ChWnd* pWnd );
		void EmptyPane( int iPane );

		void Show( int iPane, bool boolShow = true );

		void GetPaneInfo( int iPane, chint16& sIdealWidth, chint16& sMinWidth,
							chint16& sIdealHeight, chint16& sMinHeight );
		void SetPaneInfo( int iPane, chint16 sIdealWidth, chint16 sMinWidth,
							chint16 sIdealHeight, chint16 sMinHeight );

		chint16 GetSinglePane();			/* Returns index of Pane if there
												is only one Pane, otherwise -1
												is returned */

		int GetIdFromPane( int iPane ) const;
		ChPosition FindLastTopBanner() const;

											/* ClassWizard generated virtual
												function overrides */
		#if defined( CH_MSW )

		//{{AFX_VIRTUAL(ChSplitter)
		public:
		//}}AFX_VIRTUAL

											// Generated message map functions
		//{{AFX_MSG(ChSplitter)
		afx_msg void OnSize(UINT nType, int cx, int cy);
		afx_msg void OnPaint();
		afx_msg void OnMouseMove(UINT nFlags, ChPoint point);
		afx_msg void OnLButtonDown(UINT nFlags, ChPoint point);
		afx_msg void OnCancelMode();
		afx_msg void OnLButtonUp(UINT nFlags, ChPoint point);
		afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		afx_msg LONG OnCtlColorStatic( WPARAM wParam, LPARAM lParam );
		//}}AFX_MSG

#if defined(CH_PUEBLO_PLUGIN)
		afx_msg BOOL OnEraseBkgnd(CDC* pDC);
#endif
		DECLARE_MESSAGE_MAP()

		#endif	// defined( CH_MSW )

	protected:
		inline bool IsPaneInUse( int iPane ) const
						{
							ASSERT( iPane < maxPanes );
							ASSERT( iPane >= 0 );

							return m_boolPaneInUse[iPane];
						}
		inline void SetPaneInUse( int iPane, bool boolInUse = true )
						{
							ASSERT( iPane < maxPanes );
							ASSERT( iPane >= 0 );

							m_boolPaneInUse[iPane] = boolInUse;
						}
		inline void SetPaneVisible( int iPane, bool boolShow )
						{
							ASSERT( iPane < maxPanes );
							ASSERT( iPane >= 0 );

							m_boolPaneShown[iPane] = boolShow;
						}
		inline int GetPaneIndex( int iPane ) const
						{
							ASSERT( maxPanes == 2 );

							if (!m_boolSwapped)
							{
								return iPane;
							}
							else
							{
								return (0 == iPane) ? 1 : 0;
							}
						}

		void SetPaneInfo( int iPane, chint16 sCurSize, chint16 sIdealWidth,
							chint16 sMinWidth, chint16 sIdealHeight,
							chint16 sMinHeight );

		bool CountVisiblePanes();
		bool HitTest( ChPoint pt, int* piColSplitter ) const;

		#if defined( CH_MSW )

		void DeferClientPos( HDWP& hDeferWindowPos, CWnd* pWnd,
								int x, int y, int cx, int cy );
		void InvalidateSplitBar();
		void DrawAllSplitBars( CDC* pDC, int iInsideSpace );
		void OnDrawSplitter( CDC* pDC, const ChRect& rtSplitBar );
		void OnInvertTracker( const ChRect& rtTracker );
		void TrackPaneSize( int iPane, int iPos );
		void StartTracking( int iColSplitter );
		void StopTracking( bool boolAccept );

		#endif	// defined( CH_MSW )

		void GetHitRect( int iColSplitter, ChRect& rtHit );  

		#if defined( CH_ARCH_16 )
											/* These methods are part of CDC
												under win32 */
		CBrush*  GetHalftoneBrush();
		void FillSolidRect( CDC *pDC, int x, int y, int cx, int cy, COLORREF clr);
		void Draw3dRect( CDC *pDC, LPCRECT lpRect,
						COLORREF clrTopLeft, COLORREF clrBottomRight);

		#endif	// defined( CH_ARCH_16 )

		void GetSplitterCursor();

		void InitPaneSizes( const ChRect& rtClient );
		void RecalcPaneSizes( const ChRect& rtClient );
		void ResizePanes( const ChRect& rtPanes, const ChRect& rtClient );
		void RecalcFitBanners();
		void ResizeBanners( const ChRect& rtClient, HDWP& hDeferWindowPos );

		ChSplitterBanner* FindBanner( const ChString& strName,
										ChPosition& pos );

		void RecalcBannerHeights();

		void GetPaneRect( ChRect& rtPane ) const;

	protected:
		static ChString	m_strClass;

		ChRegistry		m_reg;

		ChPtrList<ChSplitterBanner>	m_bannerList;

		bool			m_boolWindows95;
		bool			m_boolDrawEdgeAvailable;

											/* Implementation attributes which
												control layout of the
												splitter */

		chint16			m_sSplitterSize;	// Width of the splitter bar
		chint16			m_sBorderShareSize;	// Space on either side of splitter
		chint16			m_sSplitterGapSize;	// Amount of space between panes
		chint16			m_sBorderSize;
		chint16			m_s3dBorderSize;

		chint32			m_lBannerTop;		// Height of top and bottom banners
		chint32			m_lBannerBottom;
		chint16			m_sBannerCount;

		COLORREF		m_colorBtnFace;
		COLORREF		m_colorBtnShadow;
		COLORREF		m_colorBtnHilite;

		HCURSOR			m_hArrow;
		HCURSOR			m_hcurSplitter;

		bool			m_boolSwapped;		// true if panes are swapped
		bool			m_boolVertical;		// true if panes are vertical

		bool			m_boolTracking;
		ChRect			m_rtTracker;
		ChPoint			m_ptTrackerOffset;
		ChRect			m_rtTrackerLimit;
		ChWnd*			m_pPanes[maxPanes];
		bool			m_boolPaneInUse[maxPanes];
		bool			m_boolPaneShown[maxPanes];
		ChPaneInfo		m_paneInfo[maxPanes];
		chint16			m_sPaneCount;

		bool			m_boolInBannerRecalc;

		#if defined( CH_ARCH_16 )

		HBRUSH			m_halftoneBrush;
		
		#endif	// defined( CH_ARCH_16 )
};

#endif	// !defined( _CHSPLIT_H )
