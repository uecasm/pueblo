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

	This file consists of the implementation of the ChArchive class.

----------------------------------------------------------------------------*/

#include "headers.h"
#include <ChArch.h>
#include <ChExcept.h>  

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#include <winsock.h>
#endif
#if defined( CH_MSW ) && (_MFC_VER >= 0x0710)
	#include <winsock2.h>
#endif

#if defined( CH_UNIX )

	#include <string.h>
	#include <minmax.h>
	#include <assert.h>
	#include <netinet/in.h>

	#define ASSERT_VALID( object )

#endif	// defined( CH_UNIX )

#include <MemDebug.h>
											// Minimum buffer size
enum { sBufSizeMin = 128 };


static chuint32 ReadStringLength( ChArchive &ar );


/*----------------------------------------------------------------------------
	Types
----------------------------------------------------------------------------*/

											/* This struct is for composing
												and decomposing a float */
typedef struct
{
	chuint8		buFloatBits[sizeof( float )];

} FloatParts;


/*----------------------------------------------------------------------------
	ChArchive constructor and destructor
----------------------------------------------------------------------------*/

ChArchive::ChArchive( ChData *pData, ChDataMode mode, chflag16 fOptions )
{
	chint32		lBufSize = 512;

	ASSERT_VALID( pData );
	ASSERT( modeRead == mode || modeWrite == mode );

											/* Initialize members not
												dependent on allocated
												buffer */
	m_mode = mode;
	m_fOptions = fOptions;
	m_pData = pData;
											/* Initialize the buffer.  Minimum
												size is 128. */
	m_boolUserBuf = true;
	m_boolDirectBuffer = false;

	m_lBufSize = sBufSizeMin;
	m_pBufStart = 0;

	lBufSize = m_lBufSize;
	if (0 == m_pBufStart)
	{										/* check for ChData providing
												buffering support */

		m_boolDirectBuffer = (m_pData->GetBufferPtr( ChData::bufferCheck ) != FALSE);
		if (!m_boolDirectBuffer)
		{									/* No support for direct buffering,
												so allocate a new buffer */
			m_pBufStart = new chuint8[m_lBufSize];
			m_boolUserBuf = false;
		}
		else
		{									/* This ChData * supports direct
												buffering! */

			lBufSize = 0;					// Will trigger initial FillBuffer
		}
	}

	if (!m_boolDirectBuffer)
	{
		ASSERT( m_pBufStart != 0);
	}

	m_pBufMax = m_pBufStart + lBufSize;
	m_pBufCur = (GetMode() & modeRead) ? m_pBufMax : m_pBufStart;

											// Seek to the start of ChData
	m_pData->Seek( ChData::begin, 0 );
}

ChArchive::~ChArchive()
{
	Abort();    // abort completely shuts down the archive
}


/*----------------------------------------------------------------------------
	ChArchive public methods
----------------------------------------------------------------------------*/

ChDataMode ChArchive::GetMode() const
{
	return m_mode;
}

ChData* ChArchive::GetData() const
{
	return m_pData;
}

void ChArchive::Abort()
{
	ASSERT( m_boolDirectBuffer || 0 != m_pBufStart );

											/* Close sets m_pData to 0.  If it
												is not 0, we must Close the
												ChArchive */

	if (0 != m_pData && !(m_fOptions & optNoFlushOnDelete))
	{
		Close();
	}

	if (!m_boolUserBuf)
	{
		ASSERT( !m_boolDirectBuffer );
		delete[] m_pBufStart;
		m_pBufStart = 0;
	}
}

void ChArchive::Close()
{
	ASSERT_VALID( m_pData );

	Flush();
	m_pData = 0;
}

void ChArchive::Flush()
{
	ASSERT_VALID( m_pData );
	ASSERT( m_boolDirectBuffer || 0 != m_pBufStart );
	ASSERT( m_boolDirectBuffer || 0 != m_pBufCur );

	if (GetMode() & modeRead)
	{
		// unget the characters in the buffer, seek back unused amount
		m_pData->Seek( -(m_pBufMax - m_pBufCur), ChData::current );
		m_pBufCur = m_pBufMax;    // empty
	}
	else
	{
		if (0 == m_pBufStart || m_pBufCur != m_pBufStart)
		{
			if (!m_boolDirectBuffer)
			{								/* write out the current buffer to
												the ChData object */

				m_pData->Write( m_pBufStart, m_pBufCur - m_pBufStart );
			}
			else
			{								// Commit the current buffer

				m_pData->GetBufferPtr( ChData::bufferCommit, m_pBufCur - m_pBufStart );
				
											// Get the next buffer

				VERIFY( m_pData->GetBufferPtr( ChData::bufferWrite, m_lBufSize,
												(void **)&m_pBufStart,
												(void **)&m_pBufMax) ==
													(chuint32)m_lBufSize );
				ASSERT( m_lBufSize == (chint32)(m_pBufMax - m_pBufStart) );
			}

			m_pBufCur = m_pBufStart;
		}
	}
}


/*----------------------------------------------------------------------------
	ChArchive reading methods
----------------------------------------------------------------------------*/

ChArchive& ChArchive::operator >>( ChStreamable& streamable )
{
	streamable.Serialize( *this );

	return *this;
}

ChArchive& ChArchive::operator >>( bool& boolVal )
{
	return operator >>( (chuint8 &)boolVal );
}

ChArchive& ChArchive::operator >>( chuint8& buVal )
{
	ASSERT( 1 == sizeof( chuint8) );		/* If this isn't true, we're
												broken */

	if (m_pBufCur + 1 > m_pBufMax)
	{										/* There is not enough data in
												the buffer */

		FillBuffer( 1 - (chuint32)(m_pBufMax - m_pBufCur) );
	}

	buVal = *m_pBufCur;						// Read the value
	m_pBufCur++;							// Move the buffer pointer

	return *this;
}

ChArchive& ChArchive::operator >>( chint8& bVal )
{
	ASSERT( sizeof( chuint8 ) == sizeof( chint8 ) );

	return operator >>( (chuint8 &)bVal );
}

ChArchive& ChArchive::operator >>( chuint16& suVal )
{
	if (m_pBufCur + sizeof( chuint16 ) > m_pBufMax)
	{
											/* There is not enough data in
												the buffer */

		FillBuffer( sizeof( chuint16 ) - (chuint32)(m_pBufMax - m_pBufCur) );
	}

	memcpy( &suVal, m_pBufCur, sizeof( chuint16 ) );
	m_pBufCur += sizeof( chuint16 );		// Move the buffer pointer

	if (!(m_fOptions & optHostByteOrder))
	{										/* Switch back from network byte
												order */
		suVal = ntohs( suVal );
	}

	return *this;
}

ChArchive& ChArchive::operator >>( chint16& sVal )
{
	ASSERT( sizeof( chuint16 ) == sizeof(chint16 ) );

	return operator >>( (chuint16 &)sVal );
}


ChArchive& ChArchive::operator >>( chuint32& luVal )
{
	if (m_pBufCur + sizeof( chuint32 ) > m_pBufMax)
	{
											/* There is not enough data in
												the buffer */

		FillBuffer( sizeof( chuint32 ) - (chuint32)(m_pBufMax - m_pBufCur) );
	}

	memcpy( &luVal, m_pBufCur, sizeof( chuint32 ) );			// Read the value
	m_pBufCur += sizeof( chuint32 );		// Move the buffer pointer

	if (!(m_fOptions & optHostByteOrder))
	{										/* Switch back from network byte
												order */
		luVal = ntohl( luVal );
	}

	return *this;
}


ChArchive& ChArchive::operator >>( chint32& lVal )
{
	ASSERT( sizeof( chuint32 ) == sizeof( chint32 ) );

	return operator >>( (chuint32 &)lVal );
}


ChArchive& ChArchive::operator >>( ChString& strVal )
{
	int			iConvert;
	chuint32	luNewLen;

	#if defined( _UNICODE )
	{
		iConvert = 1;						// If we get ANSI, convert
	}
	#else	// defined( _UNICODE )
	{
		iConvert = 0;						// If we get UNICODE, convert
	}
	#endif	// defined( _UNICODE )

	luNewLen = ReadStringLength( *this );
	if ((chuint32)-1 == luNewLen)
	{
		iConvert = 1 - iConvert;
		luNewLen = ReadStringLength( *this );

		ASSERT( (long)luNewLen != -1 );			// We shouldn't get two UNICODEs
	}

	#if defined( CH_MSW )
	{										/* The two versions of 'ChString'
												are so different in the
												subtleties that we have two
												implementations of the read
												operation... */

											/* Set length of string to new
												length */
		chuint32	luByteLen = luNewLen;

		#if defined( _UNICODE )
		{
			strVal.GetBufferSetLength( (int)luByteLen );
			luByteLen += luByteLen * (1 - nConvert);	// bytes to read
		}
		#else	// defined( _UNICODE )
		{
			luByteLen += luByteLen * iConvert;    // bytes to read
			strVal.GetBufferSetLength( (int)luByteLen );
		}
		#endif	// defined( _UNICODE )

											// Read in the characters
		if (luNewLen != 0)
		{
			char	*pBuffer;

			ASSERT( 0 != luByteLen );
											// Read new data

			pBuffer = strVal.GetBuffer( (int)luByteLen );

			if (Read( pBuffer, luByteLen) != luByteLen)	
			{
											/* Throw an end-of-file
												exception */
				#if defined( CH_EXCEPTIONS )
				{  
					#if defined( CH_MSW) && defined( CH_ARCH_16 )  
					{  
				
						THROW( new ChArchiveEx( ChEx::endOfData ) );
					}
					#else
					throw ChArchiveEx( ChEx::endOfData );
					#endif
				}
				#else	// defined( CH_EXCEPTIONS )
				{
					strVal = "";
					return *this;
				}
				#endif	// defined( CH_EXCEPTIONS )
			}

			pBuffer[luByteLen] = 0;			// Zero-terminate the buffer
		}

		strVal.ReleaseBuffer();
	}
	#elif defined( CH_UNIX )
	{
		char		*pBuffer;
		chuint32	luByteLen = luNewLen;
											/* Allocate a buffer for the 
												string */
		pBuffer = new char[luByteLen + 1];

		if (Read( pBuffer, luByteLen) != luByteLen)
		{
											/* Throw an end-of-file
												exception */
			#if defined( CH_EXCEPTIONS )
			{
				throw ChArchiveEx( ChEx::endOfData );
			}
			#else	// defined( CH_EXCEPTIONS )
			{
				strVal = "";
				return *this;
			}
			#endif	// defined( CH_EXCEPTIONS )
		}

		pBuffer[luByteLen] = 0;				// Zero-terminate the buffer

		strVal = pBuffer;					// Convert the buffer to a string
		delete[] pBuffer;
	}
	#endif

	return *this;
}


static chuint32 ReadStringLength( ChArchive &archive )
{
	chuint32	luNewLen;
	chuint8		buLen;
	chuint16	suLen;
											// First try to read a byte length
	archive >> buLen;

	if (buLen < 0xff)
	{
		return buLen;
	}
											// Now try to read a 16-bit length
	archive >> suLen;
	if (suLen == 0xfffe)
	{										/* UNICODE string prefix
												(length will follow) */
		return (chuint32)-1;
	}
	else if (suLen == 0xffff)
	{										// This is a 32-bit length
		archive >> luNewLen;

		return luNewLen;
	}
	else
	{
		return suLen;
	}
}


ChArchive& ChArchive::operator >>( float& fVal )
{
	FloatParts	parts;

	if (m_pBufCur + sizeof( float ) > m_pBufMax)
	{
											/* There is not enough data in
												the buffer */

		FillBuffer( sizeof( float ) - (chuint32)(m_pBufMax - m_pBufCur) );
	}

	*(FloatParts*)&fVal = *(FloatParts*)m_pBufCur;
	m_pBufCur += sizeof( float );

	*(float*)&parts = fVal;

	ASSERT( sizeof( float ) == 4 );

	(*(FloatParts*)&fVal).buFloatBits[0] = parts.buFloatBits[3];
	(*(FloatParts*)&fVal).buFloatBits[1] = parts.buFloatBits[2];
	(*(FloatParts*)&fVal).buFloatBits[2] = parts.buFloatBits[1];
	(*(FloatParts*)&fVal).buFloatBits[3] = parts.buFloatBits[0];

	return *this;
}


/*----------------------------------------------------------------------------
	ChArchive writing methods
----------------------------------------------------------------------------*/

ChArchive& ChArchive::operator <<( ChStreamable &streamable )
{
	streamable.Serialize( *this );

	return *this;
}

ChArchive& ChArchive::operator <<( bool boolVal )
{
	boolVal = !!boolVal;					// Ensure it's boolean

	return operator <<( (chuint8)boolVal );
}

ChArchive& ChArchive::operator <<( chuint8 buVal )
{
	ASSERT( 1 == sizeof( chuint8) );		/* If this isn't true, we're
												broken */

	if (m_pBufCur + 1 > m_pBufMax)
	{
		Flush();
	}

	*m_pBufCur = buVal;
	m_pBufCur++;

	return( *this );
}

ChArchive& ChArchive::operator <<( chint8 bVal )
{
	ASSERT( sizeof( chuint8 ) == sizeof( chint8 ) );

	return operator <<( (chuint8)bVal );
}

ChArchive& ChArchive::operator <<( chuint16 suVal )
{
	if (m_pBufCur + sizeof( chuint16 ) > m_pBufMax)
	{
		Flush();
	}

	chuint16 sTemp;
	if (!(m_fOptions & optHostByteOrder))
	{										// Switch to network byte order
		sTemp = htons( suVal );
	}
	else
	{										// No byte swapping
		sTemp = suVal;
	}
	memcpy( m_pBufCur, &sTemp, sizeof( chuint16 ) );

	m_pBufCur += sizeof( chuint16 );

	return( *this );
}

ChArchive& ChArchive::operator <<( chint16 sVal )
{
	ASSERT( sizeof( chuint16 ) == sizeof( chint16 ) );

	return operator <<( (chuint16)sVal );
}

ChArchive& ChArchive::operator <<( chuint32 luVal )
{
	if (m_pBufCur + sizeof( chuint32 ) > m_pBufMax)
	{
		Flush();
	}

	chuint32 luTemp;
	if (!(m_fOptions & optHostByteOrder))
	{										// Switch to network byte order

		luTemp = htonl( luVal );
	}
	else
	{										// No byte swapping
		luTemp = luVal;
	}
	memcpy( m_pBufCur, &luTemp, sizeof( chuint32 ) );

	m_pBufCur += sizeof( chuint32 );

	return( *this );
}

ChArchive& ChArchive::operator <<( chint32 lVal )
{
	ASSERT( sizeof( chuint32 ) == sizeof( chint32 ) );

	return operator <<( (chuint32)lVal );
}

/*----------------------------------------------------------------------------
	Strings are prefixed in the archive as follows:

	  byte: |_1____|_2____|_3____|______________________________
			| < FF |................Characters (up to 255)
			| FF   | < FFFE      |..Characters (256 - 65534)
			| FF   | FF	  | FF   |..4 bytes of length followed by characters
			| FF   | FF   | FE   |..Special prefix for UNICODE strings,
			|	   |	  |	     |	followed by normal string encoding
----------------------------------------------------------------------------*/

ChArchive& ChArchive::operator <<( ChString strVal )
{
	chint32		lStringLen;
	chint32		lByteLen;
	const char	*pBuffer;

	#if defined( CH_MSW )
	{
		lStringLen = strVal.GetLength();
	}
	#else
	{
		lStringLen = strVal.length();
	}
	#endif

	#if defined( _UNICODE )
	{
											/* Special signature to recognize
												unicode strings */
		operator << (chuint8)0xff;
		operator << (chuint16)0xfffe;

		lByteLen = lStringLen * sizeof( wchar_t );
	}
	#else	// defined( _UNICODE )
	{
		lByteLen = lStringLen * sizeof( char );
	}
	#endif	// defined( _UNICODE )

	if (lStringLen < 0xFF)
	{
		*this << (chuint8)lStringLen;
	}
	else if (lStringLen < 0xfffe)
	{
		*this << (chuint8)0xff;
		*this << (chuint16)lStringLen;
	}
	else
	{
		*this << (chuint8)0xff;
		*this << (chuint16)0xffff;
		*this << (chuint32)lStringLen;
	}

	#if defined(CH_MSW)
	{
		pBuffer = strVal.GetBuffer( 1 );
	}
	#elif defined( CH_UNIX )
	{
		pBuffer = strVal.chars();
	}
	#endif

	Write( pBuffer, lByteLen );

	return *this;
}


ChArchive& ChArchive::operator <<( float fVal )
{
	FloatParts	parts;
	chuint8*	pByte;

	if (m_pBufCur + sizeof( chuint16 ) > m_pBufMax)
	{
		Flush();
	}

	*(float*)&parts = fVal;

	ASSERT( sizeof( float ) == 4 );

	pByte = m_pBufCur;

	*pByte++ = parts.buFloatBits[3];
	*pByte++ = parts.buFloatBits[2];
	*pByte++ = parts.buFloatBits[1];
	*pByte = parts.buFloatBits[0];

	m_pBufCur += sizeof( float );

	return( *this );
}


/*----------------------------------------------------------------------------
	ChArchive public methods (should not be used by module authors)
----------------------------------------------------------------------------*/

void ChArchive::FillBuffer( chuint32 luBytesNeeded )
{
	ASSERT_VALID( m_pData );
	ASSERT( GetMode() & modeRead );
	ASSERT( m_boolDirectBuffer || 0 != m_pBufStart );
	ASSERT( m_boolDirectBuffer || 0 != m_pBufCur );
	ASSERT( luBytesNeeded > 0 );
	ASSERT( luBytesNeeded <= (chuint32)m_lBufSize );

											/* Fill up the current buffer from
												the ChData object */
	if (!m_boolDirectBuffer)
	{
		ASSERT( 0 != m_pBufCur );
		ASSERT( 0 != m_pBufStart );
		ASSERT( 0 != m_pBufMax );

		if (m_pBufCur > m_pBufStart)
		{									/* Move the currently unused part of
												the buffer to the start of the
												buffer */

			chuint32	luUnused = m_pBufMax - m_pBufCur;
			chuint32	luActual;

			if ((chint32)luUnused > 0)
			{
				ChMemMove( m_pBufStart, m_pBufCur, luUnused );
				m_pBufCur = m_pBufStart;
				m_pBufMax -= luUnused;
			}
			
			luActual = m_pData->Read( m_pBufStart + luUnused,
										m_lBufSize - luUnused );
			m_pBufCur = m_pBufStart;
			m_pBufMax = m_pBufStart + luUnused + luActual;
		}
	}
	else
	{										/* Seek to the unused portion of
												the buffer and get the buffer
												starting there */
		chuint32	luActual;

		m_pData->Seek( -(m_pBufMax - m_pBufCur), ChData::current );
		luActual = m_pData->GetBufferPtr( ChData::bufferRead, m_lBufSize,
											(void **)&m_pBufStart,
											(void **)&m_pBufMax );
		
		ASSERT( luActual == (chuint32)(m_pBufMax - m_pBufStart) );

		m_pBufCur = m_pBufStart;
	}
											// Not enough data to fill request?

	if ((chuint32)(m_pBufMax - m_pBufCur) < luBytesNeeded)
	{
											/* Throw an end-of-file
												exception */
		#if defined( CH_EXCEPTIONS )
		{
			#if defined( CH_MSW) && defined( CH_ARCH_16 )  
			{  
						
				THROW( new ChArchiveEx( ChEx::endOfData ) );
			}
			#else
			throw ChArchiveEx( ChEx::endOfData );  
			#endif
		}
		#endif	// defined( CH_EXCEPTIONS )
	}
}

chuint32 ChArchive::Read( void *pBuffer, chuint32 luMax )
{
	chuint32	luAmtToRead;
	chuint32	luTemp;

	ASSERT_VALID( m_pData );

	if (luMax == 0)
	{										// Nothing to read
		return 0;
	}

	ASSERT( 0 != pBuffer );
	ASSERT( m_boolDirectBuffer || 0 != m_pBufStart );
	ASSERT( m_boolDirectBuffer || 0 != m_pBufCur );
	ASSERT( GetMode() & modeRead );
											/* Try to fill from the buffer
												first */
	luAmtToRead = luMax;
											/* Figure out what the maximum
												we can read from the buffer
												is (this is zero if there is
												no buffer */
	
	luTemp = min( luAmtToRead, (chuint32)(m_pBufMax - m_pBufCur) );

											/* Copy the maximum number of bytes
												available (no op for non-
												buffered) */
	ChMemCopy( pBuffer, m_pBufCur, luTemp );
	m_pBufCur += luTemp;					// Move the buffer pointer
	pBuffer = (chuint8 *)pBuffer + luTemp;	/* Move the start of the read
												buffer */
	luAmtToRead -= luTemp;					// Adjust the amount still to read

	if (luAmtToRead != 0)
	{										// We still have to read data
		chuint32	luRead;

		ASSERT( m_pBufCur == m_pBufMax );

											/* Read the rest in buffer-size
												chunks */

		luTemp = luAmtToRead - (luAmtToRead % m_lBufSize);
		luRead = m_pData->Read( pBuffer, luTemp );
		pBuffer = (chuint8 *)pBuffer + luRead;
		luAmtToRead -= luRead;

											/* Read the last (odd-sized) chunk
												into buffer and then copy */
		if (luRead == luTemp)
		{
			ASSERT( m_pBufCur == m_pBufMax );
			ASSERT( luAmtToRead < (chuint32)m_lBufSize );

											/* Fill buffer (similar to
												ChArchive::FillBuffer, but no
												exception) */
			if (!m_boolDirectBuffer)
			{								// Read from ChData's buffer

				luRead = m_pData->Read( m_pBufStart,
										max( luAmtToRead,
												(chuint32)m_lBufSize ) );
				m_pBufCur = m_pBufStart;
				m_pBufMax = m_pBufStart + luRead;
			}
			else
			{								// Read from ChData directly

				luRead = m_pData->GetBufferPtr( ChData::bufferRead, m_lBufSize,
												(void **)&m_pBufStart,
												(void **)&m_pBufMax );
				ASSERT( luRead == (chuint32)(m_pBufMax - m_pBufStart) );
				m_pBufCur = m_pBufStart;
			}

											// use first part for rest of read

			luTemp = min( luAmtToRead, (chuint32)(m_pBufMax - m_pBufCur) );
			ChMemCopy( pBuffer, m_pBufCur, luTemp );
			m_pBufCur += luTemp;
			luAmtToRead -= luTemp;
		}
	}
											/* Return the amount desired minus
												the amount we still need to
												read */
	return luMax - luAmtToRead;
}

void ChArchive::Write( const void* pBuffer, chuint32 luMax )
{
	chuint32	luTemp;

	ASSERT_VALID( m_pData );

	if (0 == luMax)
	{										// Nothing to write
		return;
	}

	ASSERT( pBuffer != 0 );
	ASSERT( m_boolDirectBuffer || 0 != m_pBufStart );
	ASSERT( m_boolDirectBuffer || 0 != m_pBufCur);
	ASSERT( GetMode() & modeWrite );
											// Copy to buffer if possible

	luTemp = min( luMax, (chuint32)(m_pBufMax - m_pBufCur) );

	ChMemCopy( m_pBufCur, pBuffer, luTemp );
	m_pBufCur += luTemp;					// Advance the write buffer ptr
	pBuffer = (chuint8 *)pBuffer + luTemp;
	luMax -= luTemp;

	if (luMax > 0)
	{										// There is still data to write
		Flush();							// Flush the full buffer

											/* Write rest of the buffer in
												buffer-sized chunks */
		luTemp = luMax - (luMax % m_lBufSize);

		m_pData->Write( pBuffer, luTemp );
		pBuffer = (chuint8 *)pBuffer + luTemp;
		luMax -= luTemp;

		if (m_boolDirectBuffer)
		{									/* Adjust the direct buffer to the
												new file position */

			VERIFY( m_pData->GetBufferPtr( ChData::bufferWrite, m_lBufSize,
											(void **)&m_pBufStart,
											(void **)&m_pBufMax) ==
														(chuint32)m_lBufSize );

			ASSERT( m_lBufSize == (chint32)(m_pBufMax - m_pBufStart) );

			m_pBufCur = m_pBufStart;
		}

											/* Copy the remaining data to the
												active buffer */
		ASSERT( luMax < (chuint32)m_lBufSize );
		ASSERT( m_pBufCur == m_pBufStart );

		ChMemCopy( m_pBufCur, pBuffer, luMax );
		m_pBufCur += luMax;
	}
}
