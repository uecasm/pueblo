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

	This file consists of the interface for all the embedded objects in the 
	text view window/

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHTXTOBJ_H )
#define _CHTXTOBJ_H


#if defined( CH_MSW )
											/* Disable warnings about non-
												exported classes for MSW */
	#pragma warning( disable: 4275 )
	#pragma warning( disable: 4251 )

#include <ChScrWnd.h>

#endif	// defined( CH_MSW )

#ifdef CH_UNIX
#include <ChRect.h>
#include <ChScrlVw.h>
#include <ChDC.h>
#include <ChFont.h>
#endif  

#include <ChDibImage.h>


#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif
  

class	ChPlugInInstance;
class  	ChInlineImageData;
class  	ChInlinePluginData;
class   ChArgumentList;
class   ChTxtWnd;	   

typedef ChInlineImageData *pChInlineImageData;
typedef ChInlinePluginData *pChInlinePluginData;



/*----------------------------------------------------------------------------
MACROS :
----------------------------------------------------------------------------*/

class  ChInlineImageData
{
	public :
		ChInlineImageData( ) : m_pDIB( 0 ),
								m_iLoopCount(0),
								m_uLastUpdate(0) {}
		~ChInlineImageData();
		ChDib * GetImage()		
				{ 
					return m_pDIB; 
				}
		void    SetImage( ChDib *pDIB ) 
				{
					m_pDIB = pDIB;
				}
	public :
		int 				m_iLoopCount;
		DWORD				m_uLastUpdate;
	private :
		ChDib 				*m_pDIB;
};

class  ChInlinePluginData
{
	public :
		ChInlinePluginData( ChArgumentList* pArgs ) :  m_pluginInst(0), m_pArgList( pArgs ) {}
		~ChInlinePluginData();

		ChArgumentList* GetArgs()
						{
							return m_pArgList;
						}

		ChPlugInInstance * GetPluginInstance()		
				{ 
					return m_pluginInst; 
				}
		void    SetPluginInstance( ChPlugInInstance *pInst ) 
				{
					m_pluginInst = pInst;
				}

	private :
		ChPlugInInstance*	m_pluginInst;
		ChArgumentList*		m_pArgList;
};


class CH_EXPORT_CLASS ChTextObject
{
	public :
		enum	tagObjectTypes { objectSpace = 1, objectLine, objectImage, 
								 objectPlugin, objectControl };
		ChTextObject() : m_iObjectID( -1 ) {}
		virtual ~ChTextObject(){}
		
		// these methods should be provided by the objects defined
		virtual int  GetType() = 0;
		virtual void GetObjectSize( ChSize& objSize ) = 0; 
		virtual ChSize DrawObject( ChTxtWnd *pWnd, int x, int y, chuint32 luStyle, ChSize& viewSize ) = 0;
		virtual bool   PointInObject(  ChPoint& ptTopLeft, ChPoint& ptLoc, ChPoint *pptRel = 0  ) = 0;
		virtual chuint32 GetAttrs()		{ return m_luAttr; }

		int 	GetObjectID(  )				{ return m_iObjectID; }
		void 	SetObjectID( int iID )		{ m_iObjectID = iID; }
		void    GetSize( ChSize& objSize )	{ objSize = m_sizeObject; }
		void    GetSpaceExtra( ChRect& objSpace )	{ objSpace = m_spaceExtra; }

		void 	SetSizeInfo( const ChSize& objSize, const ChRect& objSpace)					
						{
							m_sizeObject = objSize;	
							m_spaceExtra = objSpace;	
						}
	protected :
		int			m_iObjectID;		// Object ID
		ChSize		m_sizeObject;		// object info
		chuint32	m_luAttr;			// attributes for the object  
		ChRect		m_spaceExtra;		// Extra space around the object
};

class CH_EXPORT_CLASS ChObjSpace : public ChTextObject
{
	public :
		ChObjSpace( ChSize& size, ChRect& rtSpace, chuint32 luAttr  ) 
			{
				m_sizeObject = size;
				m_luAttr	 = luAttr;
				m_spaceExtra = rtSpace;
			}
		
		virtual ~ChObjSpace() { }
		
		// these methods should be provided by the objects defined

		// Get object is called with size set to the width of the view and
		// height set to any extra height required by the Textview, usally set to zero
		// this is done so that object can scale to view width
		virtual int  GetType()				{ return ChTextObject::objectSpace; }
		virtual void GetObjectSize( ChSize& objSize );
		virtual ChSize DrawObject( ChTxtWnd *pWnd, int x, int y, chuint32 luStyle, ChSize& viewSize );
		virtual chuint32 GetAttrs()		
				{ 
					return m_luAttr; 
				}
		virtual bool   PointInObject(  ChPoint& ptTopLeft, ChPoint& ptLoc, ChPoint *pptRel = 0  ) 
		{
			return false;
		}

};

class CH_EXPORT_CLASS ChObjLine : public ChTextObject
{
	public :
		ChObjLine( ChSize& size, ChRect& rtSpace, chuint32 luAttr ) 
		{
			m_sizeObject = size;
			m_luAttr	 = luAttr;
			m_spaceExtra = rtSpace;

			if ( m_clrBtnHilight == 0 && m_clrBtnShadow == 0 )
			{  // do it once
				#ifdef CH_MSW
					m_clrBtnHilight = ::GetSysColor( COLOR_BTNHIGHLIGHT );
					m_clrBtnShadow = ::GetSysColor( COLOR_BTNSHADOW );
				#else
					extern Widget formWidget; // XXX global
					XtVaGetValues( formWidget,
								   XmNtopShadowColor, &m_clrBtnHilight,
								   XmNbottomShadowColor, &m_clrBtnShadow,
								   NULL );
	
				#endif
			}
		}
		
		virtual ~ChObjLine() {}
		
		// these methods should be provided by the objects defined
		virtual int  GetType()				{ return ChTextObject::objectLine; }
		virtual void GetObjectSize( ChSize& objSize ); 
		virtual ChSize DrawObject( ChTxtWnd *pWnd, int x, int y, chuint32 luStyle, ChSize& viewSize );
		virtual chuint32 GetAttrs()		{ return m_luAttr; }
		virtual bool   PointInObject(  ChPoint& ptTopLeft, ChPoint& ptLoc, ChPoint *pptRel = 0  ) 
		{
			return false;
		}

	private :
 		static COLORREF			m_clrBtnHilight;
 		static COLORREF			 m_clrBtnShadow; // color for line drawing

};

class CH_EXPORT_CLASS ChObjInline : public ChTextObject
{
	private :
		ChInlineImageData*		m_pImage;
		ChInlinePluginData*		m_pPlugin;
	public :
		enum tagEmbedMode { embedEmbed = 0, embedFull };

		ChObjInline( ChSize& size, ChRect& rtSpace,  chuint32 luAttr, 
					int iBorder, chuint32 luColor, ChInlineImageData *pImage  ) 
			{
				m_pPlugin 	 = 0;
				m_pImage     = pImage;
				m_sizeObject = size;
				m_spaceExtra = rtSpace;
				m_luAttr	 = luAttr;
				m_iBorder	 = iBorder;
				m_luBorderColor = luColor;
				m_iMode  = embedEmbed;

			}
	
		ChObjInline( ChSize& size, ChRect& rtSpace,  chuint32 luAttr, 
					int iBorder, chuint32 luColor, ChInlinePluginData *pPlugin  ) 
			{
				m_pPlugin 	 = pPlugin;
				m_pImage     = 0;
				m_sizeObject = size;
				m_spaceExtra = rtSpace;
				m_luAttr	 = luAttr;
				m_iBorder	 = iBorder;
				m_luBorderColor = luColor;
				m_iMode  = embedEmbed;

			}
		 virtual ~ChObjInline();

		 void SetPluginData( ChInlinePluginData* pPlugin ) 	{ m_pPlugin = pPlugin; }
		 void SetImageData( ChInlineImageData* pImage ) 	{ m_pImage = pImage; }
		 void ShutdownPlugin();
		
		// these methods should be provided by the objects defined
		virtual int  GetType()				{ return m_pPlugin ? ChTextObject::objectPlugin 
															: ChTextObject::objectImage; }
		virtual int  GetMode()				{ return m_iMode; }
		virtual void SetMode( int iMode)	{ m_iMode = iMode; }
		virtual void GetObjectSize( ChSize& objSize );
		virtual void GetImageSize( ChSize& objSize );
		virtual ChSize DrawObject( ChTxtWnd *pWnd, int x, int y, chuint32 luStyle, ChSize& viewSize );
		virtual chuint32 GetAttrs()			{ return m_luAttr; }
		virtual ChInlineImageData * GetImageData() { return m_pImage; }
		virtual ChInlinePluginData * GetPluginData() { return m_pPlugin; }
		virtual bool   PointInObject(  ChPoint& ptTopLeft, ChPoint& ptLoc, ChPoint *pptRel = 0  ); 
		void SetObjectSize( const ChSize& objSize ) { m_sizeObject = objSize; }
	private :
		int 		m_iBorder; //Border around the image
		chuint32	m_luBorderColor;
		int 		m_iMode;
};

class CH_EXPORT_CLASS ChObjControl : public ChTextObject
{
	public :
		#if defined( CH_MSW )
		ChObjControl( ChSize& size, ChRect& rtSpace,  chuint32 luAttr, CWnd* pWnd  ) 
						: m_pWnd( pWnd )
		#else
		ChObjControl( ChSize& size, ChRect& rtSpace,  chuint32 luAttr  ) 
		#endif
			{
				m_sizeObject = size;
				m_luAttr	 = luAttr;
				m_spaceExtra = rtSpace;
			}
		 ~ChObjControl() {}
		
		// these methods should be provided by the objects defined
		virtual int  GetType()				{ return ChTextObject::objectControl; }
		virtual void GetObjectSize( ChSize& size ) 
			{
				if ( m_pWnd )
				{
					CRect rtRect;
					m_pWnd->GetWindowRect( rtRect );	
					m_sizeObject.cx = rtRect.Width();
					m_sizeObject.cy = rtRect.Height();
				}
				size = m_sizeObject;
			
				size.cx += ( m_spaceExtra.left + m_spaceExtra.right );	
				size.cy += ( m_spaceExtra.top + m_spaceExtra.bottom );

			}
		virtual ChSize DrawObject( ChTxtWnd *pWnd, int x, int y, chuint32 luStyle, ChSize& viewSize );
		virtual chuint32 GetAttrs()		{ return m_luAttr; }
		virtual bool   PointInObject(  ChPoint& ptTopLeft, ChPoint& ptLoc, ChPoint *pptRel = 0  ) 
		{
			return false;
		}
		void RemoveControl()
				{
					m_pWnd = 0;
				}

	private :
		#if defined( CH_MSW )
		CWnd * m_pWnd;
		#endif
};

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR    
#endif

// $Log$

#endif // _CHTXTOBJ_H
