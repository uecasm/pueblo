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

	This file consists of the interface for the ChMemData class.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHMDATA_H )
#define _CHMDATA_H
 
#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif
 
#include <ChTypes.h>
#include <ChData.h>


/*----------------------------------------------------------------------------
	ChMemData class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChMemData : public ChData
{
	public:
		ChMemData();
		virtual ~ChMemData();

		virtual chuint32 GetSize() const;
		virtual const void* GetBuffer() { return m_pBuffer; };

		virtual void Flush();
		virtual void Close();
		virtual void Abort();
											/* The following should not be
												used directly by module
												authors */
	public:
		virtual chuint32 GetPosition() const;
		virtual chuint32 Read( void* pBuf, chuint32 luCount );
		virtual void Write( const void *pBuffer, chuint32 luCount );
		virtual chint32 Seek( chint32 lOff, chuint16 suFrom );

		virtual chuint32 ChMemData::GetBufferPtr( chuint16 suCommand,
													chuint32 luCount,
													void **ppBufStart,
													void **ppBufMax );

		virtual void SetLength( chuint32 luNewLen );

	protected:
											// Low-level functionality

		virtual void* Alloc( chuint32 luSize );
		virtual void* Realloc( void* pBlock, chuint32 luNewSize );
		virtual void* Memcpy( void *pTarget, const void *pSource, chuint32 luCount );
		virtual void Free( void *pBlock );
		virtual void GrowData( chuint32 luNewLen );

	protected:
		chuint16	m_suGrowAmount;
		chuint32	m_luPosition;
		chuint32	m_luBufferSize;
		chuint32	m_luDataSize;
		chuint8		*m_pBuffer;
};

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR    
#endif

// $Log$

#endif	// !defined( _CHMDATA_H )
