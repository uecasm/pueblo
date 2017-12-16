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

	This file consists of interface for the ChPersistentFrame class,
	which remembers the frame's location on the desktop.  This class is
	adopted from the book 'Inside Visual C++' by David J. Kruglinski.

----------------------------------------------------------------------------*/

#if (!defined( _CHPERSISTFRM_H ))
#define _CHPERSISTFRM_H

#ifdef CH_UNIX
#include <ChTypes.h>
#endif

#if defined( CH_MSW) && defined( CH_PUEBLO_PLUGIN )
#include <afxpriv.h> // for WM_MESSAGESTRING	
#endif


#if defined( CH_MSW )
											/* Disable warnings about non-
												exported classes for MSW */
	#pragma warning( disable: 4275 )
	#pragma warning( disable: 4251 )

#endif	// defined( CH_MSW )


/*----------------------------------------------------------------------------
	ChPersistentFrame class
----------------------------------------------------------------------------*/
 
#ifdef CH_MSW

#if defined( CH_PUEBLO_PLUGIN )
class CH_EXPORT_CLASS ChPersistentFrame : public CWnd
#else
class CH_EXPORT_CLASS ChPersistentFrame : public CFrameWnd
#endif

#else

class CH_EXPORT_CLASS ChPersistentFrame

#endif
{
	#if defined( CH_MSW )

    DECLARE_DYNAMIC( ChPersistentFrame )

	#endif	// defined( CH_MSW )
		 
	public:
		#if defined( CH_MSW )

		inline CWnd* GetActiveWnd() { return m_pActiveWnd; }
		inline void SetActiveWnd( CWnd* pWnd ) { m_pActiveWnd = pWnd; }

		#if defined( CH_PUEBLO_PLUGIN )
    	virtual void ActivateFrame( int nCmdShow = -1 )
		{
		}
		#else
    	virtual void ActivateFrame( int nCmdShow = -1 );
		#endif

		#endif	// defined( CH_MSW )
	
		#if (defined( CH_MSW ) && defined( CH_PUEBLO_PLUGIN ))

	    inline void SetMessageText( const char* pstrText )
			    {
					SendMessage( WM_SETMESSAGESTRING, 0, (LPARAM)pstrText );
			    }

		#elif defined( CH_UNIX )
	
		void SetMessageText(const char* pstrText );

		#endif	// defined( CH_UNIX )
	
		inline const ChString& GetLabel() const { return m_strLabel; }
		
		void SetLabel( const ChString& strLabel );

	protected:
		#if defined( CH_MSW )

	#if !defined( CH_PUEBLO_PLUGIN )
		inline const ChString& GetRegSection() const { return m_strRegSection; }
	#endif

		inline void RememberFocus()
				{
					HWND	hwndCurrFocus = ::GetFocus();

					if (0 != hwndCurrFocus)
					{
						m_hwndFocus = hwndCurrFocus;
					}
				}

		inline void RestoreFocus()
				{
					if ((0 != m_hwndFocus) && IsWindow( m_hwndFocus ))
					{
						::SetFocus( m_hwndFocus );
					}
				}

		#endif	// defined( CH_MSW )

	private:
		#if defined( CH_MSW )  
		
		#if !defined( CH_PUEBLO_PLUGIN )		
		
		static const char	cRegistrySeparator;
		static const ChString	strProfileRect;
		static const ChString	strProfileIcon;
		static const ChString	strProfileMax;
		static const ChString	strProfileTool;
		static const ChString	strProfileStatus;

 		ChString				m_strRegSection;

		#endif

		CWnd*				m_pActiveWnd;
		HWND				m_hwndFocus;

		#endif // CH_MSW

		ChString				m_strLabel;
		bool				boolFirstTime;
		bool				boolIconic;

	protected:
											/* This class may only be created
												from serialization */
    	ChPersistentFrame();

		#if defined( CH_MSW )

	    //{{AFX_MSG(ChPersistentFrame)
		#if !defined( CH_PUEBLO_PLUGIN )		
	    afx_msg void OnDestroy();
		afx_msg void OnActivate( UINT nState, CWnd* pWndOther, BOOL boolMinimized );
		afx_msg void OnSysCommand( UINT nID, LONG lParam );
		#endif
		//}}AFX_MSG

    	DECLARE_MESSAGE_MAP()

		#endif	// defined( CH_MSW )
};



#endif	/* !defined( _CHPERSIST_H ) */

// Local Variables: ***
// tab-width:4 ***
// End: ***
