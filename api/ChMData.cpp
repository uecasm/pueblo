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

#include "headers.h"
#include <ChMData.h>
#include <ChExcept.h>

#if defined( CH_UNIX )

	#include <minmax.h>
	#include <malloc.h>
	
	#define ASSERT_VALID( object )

#endif	// defined( CH_UNIX )

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	ChMemData Constructor and Destructor
----------------------------------------------------------------------------*/

ChMemData::ChMemData() : ChData( modeReadWrite )
{
	m_suGrowAmount = 1024;
	m_luPosition = 0;
	m_luBufferSize = 0;
	m_luDataSize = 0;
	m_pBuffer = 0;
	m_luBufferSize = 0;
}

ChMemData::~ChMemData()
{
	if (m_pBuffer)
	{										/* Ensure that the memory data has
												been closed */
		Close();
	}
	ASSERT( 0 == m_pBuffer );

	m_suGrowAmount = 0;
	m_luPosition = 0;
	m_luBufferSize = 0;
	m_luDataSize = 0;
}


/*----------------------------------------------------------------------------
	ChMemData public methods
----------------------------------------------------------------------------*/

void ChMemData::Flush()
{
	ASSERT_VALID( this );
}

void ChMemData::Close()
{
	ASSERT_VALID( this );

	m_suGrowAmount = 0;
	m_luPosition = 0;
	m_luBufferSize = 0;
	m_luDataSize = 0;

	if (m_pBuffer)
	{										// Free any data allocated
		Free( m_pBuffer );
	}
	m_pBuffer = 0;
}

void ChMemData::Abort()
{
	ASSERT_VALID( this );

	Close();
}


/*----------------------------------------------------------------------------
	ChMemData public methods (should not be used by module authors)
----------------------------------------------------------------------------*/

chuint32 ChMemData::GetSize() const
{
	return m_luDataSize;
}

chuint32 ChMemData::GetPosition() const
{
	return m_luPosition;
}

chuint32 ChMemData::Read( void* pBuffer, chuint32 luCount )
{
	chuint32	luBytesRead;

	ASSERT_VALID( this );

	if (0 == luCount)
	{										// We're not reading anything
		return 0;
	}

	ASSERT( 0 != pBuffer );

	if (m_luPosition > m_luDataSize)
	{										// We're positioned past the end
		return 0;
	}

	if (m_luPosition + luCount > m_luDataSize)
	{										/* We can only read the number of
												bytes remaining, which is less
												than they asked for */

		luBytesRead = m_luDataSize - m_luPosition;
	}
	else
	{
		luBytesRead = luCount;
	}

	Memcpy( (chuint8 *)pBuffer, m_pBuffer + m_luPosition, luBytesRead );
	m_luPosition += luBytesRead;				// Move the current position

	ASSERT_VALID( this );

	return luBytesRead;
}

void ChMemData::Write( const void *pBuffer, chuint32 luCount )
{
	ASSERT_VALID( this );

	if (0 == luCount)
	{										// Nothing to write!
		return;
	}

	ASSERT( 0 != pBuffer );

	if (m_luPosition + luCount > m_luBufferSize)
	{										// The storage isn't large enough
		GrowData( m_luPosition + luCount );
	}

	ASSERT( m_luPosition + luCount <= m_luBufferSize );

	Memcpy( m_pBuffer + m_luPosition, (chuint8 *)pBuffer, luCount );
	m_luPosition += luCount;				// Move the current position

	if (m_luPosition > m_luDataSize)
	{										/* If the position is past the end
												of the data block (?) correct
												things */
		m_luDataSize = m_luPosition;
	}

	ASSERT_VALID( this );
}

chint32 ChMemData::Seek( chint32 lOff, chuint16 suFrom )
{
	chint32 lNewPos = m_luPosition;

	ASSERT_VALID( this );

	ASSERT( suFrom == begin || suFrom == end || suFrom == current);

	if (suFrom == begin)
	{
		lNewPos = lOff;
	}
	else if (suFrom == current)
	{
		lNewPos += lOff;
	}
	else if (suFrom == end)
	{
		lNewPos = m_luDataSize + lOff;
	}
	else
	{										// Error!
		return -1;
	}

	if (lNewPos < 0)
	{										// Seek failed badly...

		#if defined( CH_EXCEPTIONS )
		{
			#if defined( CH_MSW) && defined( CH_ARCH_16 )  
			{  
				THROW( new ChDataEx( ChEx::badSeek ) );
			}
			#else
			throw ChDataEx( ChEx::badSeek );
			#endif
		}
		#else	// defined( CH_EXCEPTIONS )
		{
			return -1 ;
		}
		#endif	// defined( CH_EXCEPTIONS )
	}

	m_luPosition = lNewPos;

	ASSERT_VALID( this );

	return m_luPosition;
}

chuint32 ChMemData::GetBufferPtr( chuint16 suCommand, chuint32 luCount,
									void **ppBufStart, void **ppBufMax )
{
											/* ChMemData supports "direct
												buffering" interaction with
												ChArchive */

	ASSERT( suCommand == bufferCheck || suCommand == bufferCommit ||
			suCommand == bufferRead || suCommand == bufferWrite );

	if (suCommand == bufferCheck)
	{
		return 1;							/* Just a check for direct buffer
												support */
	}

	if (suCommand == bufferCommit)
	{										// Commit buffer
		ASSERT( 0 == ppBufStart );
		ASSERT( 0 == ppBufMax );

		m_luPosition += luCount;

		if (m_luPosition > m_luDataSize)
		{
			m_luDataSize = m_luPosition;
		}

		return 0;
	}
											/* When storing, grow file as
												necessary to satisfy buffer
												request */

	if ((suCommand == bufferWrite) && (m_luPosition + luCount > m_luBufferSize))
	{
		GrowData( m_luPosition + luCount );
	}

	ASSERT( suCommand == bufferWrite || suCommand == bufferRead );

											// Store buffer max and min
	ASSERT( 0 != ppBufStart );
	ASSERT( 0 != ppBufMax );

	*ppBufStart = m_pBuffer + m_luPosition;
	*ppBufMax = m_pBuffer + min( m_luBufferSize, m_luPosition + luCount );

											/* Advance current file position
												only on read */
	if (bufferRead == suCommand)
	{
		m_luPosition += (chuint8 *)*ppBufMax - (chuint8 *)*ppBufStart;
	}

											/* Return number of bytes in
												returned buffer space (may be
												<= luCount) */

	return (chuint8 *)*ppBufMax - (chuint8 *)*ppBufStart;
}

void ChMemData::SetLength( chuint32 luNewLen )
{
	ASSERT_VALID( this );

	if (luNewLen > m_luBufferSize)
	{
		GrowData( luNewLen );
	}

	if (luNewLen < m_luPosition)
	{
		m_luPosition = luNewLen;
	}

	m_luDataSize = luNewLen;

	ASSERT_VALID( this );
}


/*----------------------------------------------------------------------------
	ChMemData low-level functionality (protected)
----------------------------------------------------------------------------*/

void* ChMemData::Alloc( chuint32 luSize )
{
	return (void *)malloc( (size_t)luSize );
}

void* ChMemData::Realloc( void *pBlock, chuint32 luNewSize )
{
	return (void *)realloc( pBlock, (size_t)luNewSize );
}

//#pragma intrinsic (memcpy)

void* ChMemData::Memcpy( void *pTarget, const void *pSource, chuint32 luCount )
{
	ASSERT( 0 != pTarget );
	ASSERT( 0 != pSource );

	return (void *)ChMemCopy( pTarget, pSource, luCount );
}
#pragma function( memcpy )

void ChMemData::Free( void *pBlock )
{
	ASSERT( 0 != pBlock );

	free( pBlock );
}

void ChMemData::GrowData( chuint32 luNewLen )
{
	ASSERT_VALID( this );

	if (luNewLen > m_luBufferSize)
	{										// Grow the buffer
		chuint8 *pNew;

		chuint32	luNewBufSize = m_luBufferSize;

		while (luNewBufSize < luNewLen)
		{
			luNewBufSize += m_suGrowAmount;
		}

		if (0 == m_pBuffer)
		{
			pNew = (chuint8 *)Alloc( luNewBufSize );
		}
		else
		{
			pNew = (chuint8 *)Realloc( m_pBuffer, luNewBufSize );
		}

		if (0 == pNew)
		{									// Out of memory
			#if defined( CH_EXCEPTIONS )
			{
				#if defined( CH_MSW) && defined( CH_ARCH_16 )  
				{  
						
					THROW( new ChMemEx() );
				}
				#else
				throw ChMemEx();  
				#endif
			}
			#endif	// defined( CH_EXCEPTIONS )
		}

		m_pBuffer = pNew;
		m_luBufferSize = luNewBufSize;
	}

	ASSERT_VALID( this );
}

// $Log$
