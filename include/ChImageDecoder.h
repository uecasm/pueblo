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

	Header file for the ChImageDecoder class.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHIMGDECODER_H )
#define _CHIMGDECODER_H

typedef int LZHANDLE;

class ChImageConsumer;

class CH_EXPORT_CLASS ChImageDecoder
{
	public:
	    ChImageDecoder( ChImageConsumer* pConsumer ) :
							m_pImgConsumer( pConsumer ),
							m_flOptions( 0 )
	    				{}
	    virtual ~ChImageDecoder() {}

		enum tagOptions { loadAuto = 0x01, load8Bit = 0x02, load24Bit = 0x04 };

									// Load from disk file
	    virtual bool Load(const char* pszFileName = NULL, chuint32 flOption = loadAuto ) = 0;
									// Load from resource
	    virtual bool Load(WORD wResid, HINSTANCE hInst = 0) = 0; 
									// Load DIB from LZ File
	    virtual bool Load(LZHANDLE lzHdl) = 0; 
	    
	    virtual bool ProcessImageData( const BYTE* pBits, chuint32 luBytes ) 
	    				{
							return false;
	    				}; 
	    
	    inline  ChImageConsumer* GetConsumer() { return m_pImgConsumer; }
	    inline  void SetConsumer( ChImageConsumer* pConsumer )
	    					{
								m_pImgConsumer = pConsumer;	
	    					}           
					
	protected:

		ChImageConsumer*	m_pImgConsumer;
		chuint32			m_flOptions;

};

// $Log$

#endif //  !defined( _CHIMGDECODER_H )
