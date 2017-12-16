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

	This file contains the interface for ChCore, a base class used
	by ChServerCore and ChClientCore.

----------------------------------------------------------------------------*/

#if (!defined( _CHTIME_H ))
#define _CHTIME_H

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif

#include <ChStrmbl.h>

typedef double ChTime_t;
class ChTimeSpan;

/*----------------------------------------------------------------------------
	ChTime class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChTime : public ChStreamable
{

	public:
		ChTime() : m_dTime( 0 ) {}
		ChTime( ChTime_t t ) : m_dTime(t) {};
		ChTime( const char* pstrUnivTime );

		ChTime( const ChTime& srcTime ) :  m_dTime(srcTime.m_dTime){};


		ChTime( chint16 sYear, chint16 sMonth, chint16 sDay, 
				chint16 sHour, chint16 sMin, chint16 sSec );


		static ChTime GetCurrentTime();        
		
		struct tm* GetLocalTm( struct tm* ptm = NULL );

		virtual void Serialize( ChArchive &archive );

		ChString Format( const char *pstrFormat = 0);

		chint16 GetDay() const;

		chint16 GetDayOfWeek() const;

		chint16 GetHour() const;

		chint16 GetMinute() const;
		chint16 GetMonth() const;
		chint16 GetSecond() const;
		inline ChTime_t GetTime() const
				{
					return m_dTime;
				}
		chint16 GetYear() const;

		const ChTime& operator =( const ChTime_t time );


		ChTimeSpan operator-(ChTime time) const;
		ChTime operator-(ChTimeSpan timeSpan) const;
		ChTime operator+(ChTimeSpan timeSpan) const;
		const ChTime& operator+=(ChTimeSpan timeSpan);
		const ChTime& operator-=(ChTimeSpan timeSpan);
		bool operator==(ChTime time) const;
		bool operator!=(ChTime time) const;
		bool operator<(ChTime time) const;
		bool operator>(ChTime time) const;
		bool operator<=(ChTime time) const;
		bool operator>=(ChTime time) const;



	protected:
		//void ConvertFromSysTime( time_t sysTime );
		int GetMonth( const ChString& strMonth );


	protected:
		ChTime_t	m_dTime;		// based in 1970, but may be negative or big
};


/*----------------------------------------------------------------------------
	ChTimeSpan class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChTimeSpan : public ChStreamable
{

	public:
		ChTimeSpan() : m_span(0)  {}
		ChTimeSpan( ChTime_t t ) : m_span(t) {}
		ChTimeSpan( const ChTimeSpan& span ) : m_span(span.m_span) {};
		ChTimeSpan( chint32 lDays, chint16 sHours, chint16 sMins, chint16 sSecs );

		virtual void Serialize( ChArchive &archive );
		ChString Format( const char *pstrFormat = 0);

		chint32 GetDays() const;
		chint16 GetHours() const;
		chint16 GetMinutes() const;
		chint16 GetSeconds() const;
		chint32 GetTotalHours() const;
		chint32 GetTotalMinutes() const;
		inline double GetTotalSeconds() const { return double(m_span); }

		ChTimeSpan operator +( ChTimeSpan span ) const;
		ChTimeSpan& operator +=( ChTimeSpan span );
		ChTimeSpan operator -( ChTimeSpan span ) const;
		ChTimeSpan& operator -=( ChTimeSpan span );
 		ChTime operator+(ChTime time) const;
		bool operator==(ChTimeSpan span) const;
		bool operator!=(ChTimeSpan span) const;
		bool operator<(ChTimeSpan span) const;
		bool operator>(ChTimeSpan span) const;
		bool operator<=(ChTimeSpan span) const;
		bool operator>=(ChTimeSpan span) const;

	protected:
		ChTime_t	m_span;

};

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR    
#endif

#endif	// !defined( _CHTIME_H )

// Local Variables: ***
// tab-width:4 ***
// End: ***
