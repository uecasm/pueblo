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

	This file consists of the interface for the ChData class.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHDATA_H )
#define _CHDATA_H

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif

#include <ChTypes.h>

#if defined( CH_MSW )
											/* Disable warnings about non-
												exported classes for MSW */
	#pragma warning( disable: 4275 )
	#pragma warning( disable: 4251 )

#endif	// defined( CH_MSW )
											/* Note that in the following,
												modeReadWrite must be an
												OR of modeRead and ModeWrite */

typedef enum tagChDataMode { modeRead = 1, modeWrite = 2,
								modeReadWrite = 3 } ChDataMode;


/*----------------------------------------------------------------------------
	ChData class
----------------------------------------------------------------------------*/

#if defined( CH_MSW )
class CH_EXPORT_CLASS ChData : public CObject
#else
class CH_EXPORT_CLASS ChData
#endif
{
	public:
		virtual ~ChData();

		virtual chflag16 GetMode();

		virtual void Flush();
		virtual void Close();
		virtual void Abort();
											/* The following should not be
												used directly by module
												authors */
	public:
		enum tagBufferCmd { bufferRead, bufferWrite, bufferCommit,
							bufferCheck };
		enum tagSeekPos { begin, current, end };

	public:
		virtual chuint32 GetPosition() const;
		virtual chuint32 Read( void* pBuf, chuint32 luCount );
		virtual void Write( const void *pBuffer, chuint32 luCount );
		virtual chint32 Seek( chint32 lOff, chuint16 suFrom );

		virtual chuint32 ChData::GetBufferPtr( chuint16 suCommand,
													chuint32 luCount = 0,
													void **ppBufStart = 0,
													void **ppBufMax = 0 );

		virtual void SetLength( chuint32 luNewLen );

	protected:
		ChData( ChDataMode mode );

	protected:
		ChDataMode		m_mode;
};

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR    
#endif

// $Log$

#endif	// !defined( _CHDATA_H )
