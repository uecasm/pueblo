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

	This file consists of the interface for the ChVersion class.

----------------------------------------------------------------------------*/

#if (!defined( _CHVERS_H ))
#define _CHVERS_H

#include <ChTypes.h>
#include <ChStrmbl.h>

#if defined( CH_MSW )
											/* Disable warnings about non-
												exported classes for MSW */
	#pragma warning( disable: 4275 )
	#pragma warning( disable: 4251 )

#endif	// defined( CH_MSW )

#ifdef CH_UNIX
#define LOWORD(foo) ((foo) & 0x0000ffff)
#define HIWORD(foo) (((foo) & 0xffff0000) >> 16)
#define MAKELONG(lo, hi) (((hi) << 16) | (lo))
#endif


/*----------------------------------------------------------------------------
	ChVersion class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChVersion : public ChStreamable
{
	public:
		enum tagFormatOptions { formatNormal = 0, formatShort };

	public:
		ChVersion() : m_sMajor( 0 ), m_sMinor( 0 ), m_strLabel( "" ) {}

		ChVersion( chint16 sMajor, chint16 sMinor, char *pstrLabel = 0 ) :
						m_sMajor( sMajor ),
						m_sMinor( sMinor )
					{
						if (pstrLabel)
						{
							m_strLabel = pstrLabel;
						}
					}

		ChVersion( const ChVersion& src ) :
						m_sMajor( src.GetMajor() ),
						m_sMinor( src.GetMinor() ),
						m_strLabel( src.GetLabel() )
					{
					}

		ChVersion( chuint32 luPackedVersion ) :
						m_sMajor( HIWORD( luPackedVersion ) ),
						m_sMinor( LOWORD( luPackedVersion ) )
					{
					}

		inline void Set( chint16 sMajor, chint16 sMinor, char *pstrLabel = 0 )
					{
						m_sMajor = sMajor;
						m_sMinor = sMinor;

						if (pstrLabel)
						{
							m_strLabel = pstrLabel;
						}
						else
						{
							m_strLabel = "";
						}
					}

		inline bool operator ==( const ChVersion& versCompare ) const
					{
						return (m_sMajor == versCompare.GetMajor()) &&
								(m_sMinor == versCompare.GetMinor());
					}
		inline bool operator !=( const ChVersion& versCompare ) const
					{
						return !(*this == versCompare);
					}
		inline bool operator >( const ChVersion& versCompare ) const
					{
						return !(*this < versCompare) &&
								!(*this == versCompare);
					}
		inline bool operator <=( const ChVersion& versCompare ) const
					{
						return (*this < versCompare) || (*this == versCompare);
					}
		inline bool operator >=( const ChVersion& versCompare ) const
					{
						return !(*this < versCompare);
					}

		bool operator <( const ChVersion& versCompare ) const;

		void operator =( double version );
		void operator =( chuint32 luPackedVersion );

		inline ~ChVersion() {}

		void Serialize( ChArchive& ar );

    	inline int GetMajor() const { return m_sMajor; }
    	inline int GetMinor() const { return m_sMinor; }
    	inline chuint32 GetPacked() const
    			{
    				return MAKELONG( m_sMinor, m_sMajor );
    			}

    	inline ChString GetLabel() const
    				#if !defined( CH_ARCH_16 )
    				{ return( ChString( m_strLabel ) ); }
    				#else
   					{ return(  m_strLabel ); }
    				#endif

    	ChString Format( int iOption = formatNormal,
    					int iDecimalPlaces = 2 ) const;

	private:
		chint16		m_sMajor;
		chint16		m_sMinor;
		ChString		m_strLabel;

		// XXX Need comparison operators.
};

#endif	// !defined( _CHVERS_H )

// Local Variables: ***
// tab-width:4 ***
// End: ***
