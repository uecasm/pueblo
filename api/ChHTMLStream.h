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

	This file contains the definition of the ChHTMLStreamManager class.

----------------------------------------------------------------------------*/

#if (!defined( CHTMLSTREAM_H ))
#define CHTMLSTREAM_H

#include <ChHTTPStream.h>


class ChDib;
class ChHtmlWnd;
class ChHtmlView;
class ChObjInline;
class ChPlugInStream;
/*----------------------------------------------------------------------------
	ChHTMLLoadInfo class
----------------------------------------------------------------------------*/

class ChHtmlReqInfo
{

	public:
		enum tagType { loadError = 0, loadURL , loadInline, loadBkPattern, loadPlugInReq, loadFrameReq };

		ChHtmlReqInfo(  ChHtmlView* pView );

		virtual ~ChHtmlReqInfo() {}
		virtual chuint32 GetType() = 0;
		inline chuint32 GetReqID() 			{ return m_uReqID; }
		inline chuint32 GetViewID() 		{ return m_uViewID; }
		inline ChString&  GetViewName() 		{ return m_strViewName; }
		inline ChHtmlWnd*  GetFrameMgr() 	{ return m_pFrameMgr; }

		inline const ChString& 	GetURL( )		{  return m_strURL; } 
		inline const ChString& 	GetMimeType( )	{  return m_strMimeType; } 
		inline const ChString& 	GetFile(  )		{  return m_strData; } 
		inline const ChString& 	GetErrorMsg(  )	{  return m_strData; } 
		inline int 				GetError( )		{  return m_iError; } 

		inline void 	SetURL( const ChString& strURL )			{  m_strURL = strURL; } 
		inline void 	SetMimeType( const ChString& strMimeType)	{  m_strMimeType = strMimeType; } 
		inline void 	SetFile( const ChString& strFile )		{  m_strData = strFile; } 
		inline void 	SetError( const ChString& strMsg, int iErr )		
										{  
											m_strData = strMsg; 
											m_iError  = iErr;
										} 
		void	    	SetFrameMgrInfo( ChHtmlWnd* pWnd );

	protected :
		chuint32		m_uReqID;
		chuint32		m_uViewID;
		ChString 			m_strViewName;
		ChHtmlWnd*		m_pFrameMgr;
		// Load Info
		ChString			m_strURL;		// URL
		ChString			m_strMimeType;
		ChString			m_strData;	  	// Cache file name or error msg
		int				m_iError;
};


class ChHtmlURLReq : public ChHtmlReqInfo
{
	public :
		ChHtmlURLReq(  ChHtmlView* pView, const ChString& strTarget ) :	
								ChHtmlReqInfo( pView ),
								m_strTarget( strTarget )
				{
				}
		virtual ~ChHtmlURLReq()				{}
 
 		virtual chuint32 GetType() 			{ return loadURL; }
 
 		inline ChString&  GetTarget() 		{ return m_strTarget; }
		inline void 	SetTarget( const ChString& strTarget )	
											{  m_strTarget = strTarget; } 
	private :
		ChString			m_strTarget;
};

class ChHtmlInlineReq : public ChHtmlReqInfo
{
	public :
		ChHtmlInlineReq(  ChHtmlView* pView, ChObjInline* pInline ) :	ChHtmlReqInfo( pView ),
								m_pInline( pInline ),
								m_pDib( 0 )
				{
				}
		virtual ~ChHtmlInlineReq();

 		virtual chuint32 GetType() 			{ return loadInline; }
 
		ChObjInline* GetInlineData()		{ return  m_pInline; }
		ChDib* GetImage()					{ return  m_pDib; }
		void   SetImage( ChDib* pDib)		{ m_pDib = pDib; }
	private :
			ChObjInline*  m_pInline;
			ChDib*		  m_pDib;
};

class ChHtmlBkPatternReq : public ChHtmlReqInfo
{
	public :
		ChHtmlBkPatternReq(  ChHtmlView* pView ) :	ChHtmlReqInfo( pView ),
								m_pDib( 0 )
				{
				}
		virtual ~ChHtmlBkPatternReq();

 		virtual chuint32 GetType() 			{ return loadBkPattern; }
 
		ChDib* GetImage()					{ return  m_pDib; }
		void   SetImage( ChDib* pDib)		{ m_pDib = pDib; }
	private :
			ChDib*		  m_pDib;
};


class ChHtmlPluginReq : public ChHtmlReqInfo
{
	public :
		ChHtmlPluginReq(  ChHtmlView* pView, ChPlugInStream* pStream ) : ChHtmlReqInfo( pView ),
								m_pStream( pStream )
				{
				}
		virtual ~ChHtmlPluginReq() {}

 		virtual chuint32 GetType() 			{ return loadPlugInReq; }
 
		ChPlugInStream* GetPlugInStream()		{ return  m_pStream; }
	private :
			ChPlugInStream* 	m_pStream;
};

class ChHtmlFrameReq : public ChHtmlReqInfo
{
	public :
		ChHtmlFrameReq( ChHtmlView* pView, chuint32 luOptions ) : ChHtmlReqInfo( pView ),
								m_luOptions( luOptions )
				{
				}
		virtual ~ChHtmlFrameReq() {}

 		virtual chuint32 GetType() 			{ return loadFrameReq; }

		chuint32 GetOptions()				{ return  m_luOptions; }
	private :
			chuint32  	m_luOptions;
};




class CH_EXPORT_CLASS ChHTMLStreamManager  :  public  ChHTTPStreamManager
{
	public :
		ChHTMLStreamManager( ChHtmlWnd* pWnd) : m_htmlWnd( pWnd )
						{
						}
		virtual ~ChHTMLStreamManager() 		
						{
						}

	    virtual int NewStream( chparam requestData, pChHTTPStream pStream, bool boolSeekable  );
	    virtual chint32 WriteReady( chparam requestData, pChHTTPStream pStream, chint32 iBytes );
	    virtual chint32 Write( chparam requestData, pChHTTPStream pStream, chint32 lOffset, chint32 lLen,
	    												 const char* pBuffer );
	    virtual void DestroyStream( chparam requestData, pChHTTPStream pStream, int iReason );
	 	virtual void StreamAsFile(chparam requestData, pChHTTPStream pStream, const char* fname);

	private :
		inline ChHtmlWnd* GetHTMLWnd() 		{ return m_htmlWnd; }
		ChHtmlWnd*		m_htmlWnd;

};

#endif // (!defined( CHTMLSTREAM_H ))

