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

	This file consists of the interface for the ChArchive class.

----------------------------------------------------------------------------*/

// $Header$

#if (!defined( _CHARCH_H ))
#define _CHARCH_H

#include <ChTypes.h>
#include <ChData.h>
#include <ChStrmbl.h>   

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif


/*----------------------------------------------------------------------------
	ChArchive class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChArchive
{
	public:
		ChArchive( ChData *pData, ChDataMode mode, chflag16 fOptions = 0 );
		~ChArchive();

		ChDataMode GetMode() const;
		ChData* GetData() const;

		void Abort();
		void Close();
		void Flush();

	public:
											/* Writing operations.
												NOTE: operators available only
												for fixed size types for
												portability. */

		ChArchive& operator <<( ChStreamable &streamable );
		ChArchive& operator <<( bool boolVal );
		ChArchive& operator <<( chuint8 buVal );
		ChArchive& operator <<( chint8 bVal );
		ChArchive& operator <<( chuint16 suVal );
		ChArchive& operator <<( chint16 sVal );
		ChArchive& operator <<( chuint32 suVal );
		ChArchive& operator <<( chint32 sVal );
		ChArchive& operator <<( ChString strVal );
		ChArchive& operator <<( float fVal );

											// Reading operations

		ChArchive& operator >>( ChStreamable &streamable );
		ChArchive& operator >>( bool& boolVal );
		ChArchive& operator >>( chuint8& buVal );
		ChArchive& operator >>( chint8& bVal );
		ChArchive& operator >>( chuint16& suVal );
		ChArchive& operator >>( chint16& sVal );
		ChArchive& operator >>( chuint32& suVal );
		ChArchive& operator >>( chint32& sVal );
		ChArchive& operator >>( ChString& strVal );
		ChArchive& operator >>( float& fVal );

	public:
											/* Object I/O is pointer based to
												avoid added construction
												overhead.  Use the Serialize
												member function directly for
												embedded objects. */

		friend ChArchive& operator<<( ChArchive& ar,
											const ChStreamable *pObject );
		friend ChArchive& operator>>( ChArchive& ar,
											ChStreamable*& pObject );
		friend ChArchive& operator>>( ChArchive& ar,
											const ChStreamable*& pObject );

	public:
											/* These functions shouldn't be
												used by module authors */

		void FillBuffer( chuint32 luBytesNeeded );

											/* The following functions should
												not be used as they don't
												guarantee data portability */

		chuint32 ChArchive::Read( void *pBuffer, chuint32 luMax );
		void Write( const void* pBuffer, chuint32 luMax );

	public:
											// Internal flags

		enum tagArchiveMode { optNoFlushOnDelete = 1,
								optHostByteOrder = 2 };

	protected:
											/* ChArchive objects cannot be
												copied or assigned */
		ChArchive( const ChArchive& src );
		void operator=( const ChArchive& src );

		bool			m_boolDirectBuffer;	/* true if the ChData object
												supports direct buffering */
		chflag16		m_fOptions;
		ChDataMode		m_mode;
		bool			m_boolUserBuf;
		chint32			m_lBufSize;
		ChData			*m_pData;
		chuint8			*m_pBufCur;	
		chuint8			*m_pBufMax;
		chuint8			*m_pBufStart;
};

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR 
#endif

// $Log$

#endif	// !defined( _CHARCH_H )
