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

	This file contains the implementation of the ChTime and ChTimeSpan
	classes, used for manipulating times & dates.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include <ChStrmbl.h>
#include <ChData.h>
#include <ChArch.h>
#include <ChTime.h>
#include <time.h>
// we get a bogus warning on MSVC from math.h
#pragma warning( disable: 4091 ) 
#if defined (CH_ARCH_16 )
#define __STDC__
#define MAXLONG     0x7fffffff  
#endif
#include <math.h>
#if defined (CH_ARCH_16 )
#undef __STDC__
#endif
#if !defined( CH_MSW )
#include <values.h>
#endif

#include <MemDebug.h>

#define TICKSPERLEAPPERIOD	 	(60. * 60. * 24. * (365. * 4. + 1))
#define MAXTIMETYEAR			138
#define MAXTIMEBUFSIZE			256

#define SECONDSPERMINUTE	(chint32(60))
#define SECONDSPERHOUR		(chint32(60) * 60)
#define SECONDSPERDAY		(chint32(24) * 60 * 60)
#define MINUTESPERDAY		(chint32(24) * 60)
#define HOURSPERDAY			(chint32(24))

#if !defined CH_MSW
typedef char TCHAR;
#define _istlead(a)		true
#endif

// Helper functions
static inline chint16 NormalizeYear( chint16 sYear, chint32& leaps );
static inline ChTime_t LeapsToChTime_t(chint32 leaps);
static inline chint32 GetLo32(ChTime_t t);
static inline chint32 GetHi32(ChTime_t t);
static inline ChTime_t MakeChTime_t(chint32 lo, chint32 hi);
static inline time_t NormalizeTime( double dTime, chint32 &leaps );
static bool	GetLocTm( double dTime, struct tm *pTm );
static inline double pmod( double a, double div );

CH_INTERN_VAR const char* pstrMonths[] =
				{
					TEXT( "Jan" ),
					TEXT( "Feb" ),
					TEXT( "Mar" ),
					TEXT( "Apr" ),
					TEXT( "May" ),
					TEXT( "Jun" ),
					TEXT( "Jul" ),
					TEXT( "Aug" ),
					TEXT( "Sep" ),
					TEXT( "Oct" ),
					TEXT( "Nov" ),
					TEXT( "Dec" ),
				};

/*----------------------------------------------------------------------------
	ChTime class
----------------------------------------------------------------------------*/

ChTime::ChTime( chint16 sYear, chint16 sMonth, chint16 sDay, 
				chint16 sHour, chint16 sMin, chint16 sSec )
{
	struct tm	theTime;
	chint32		leaps;	// number of extra 4 year periods
	theTime.tm_sec		=	sSec;	
	theTime.tm_min		=	sMin;	
	theTime.tm_hour		=	sHour;
	theTime.tm_mday		=	sDay;
	theTime.tm_mon		=	sMonth;	
	theTime.tm_year		=	NormalizeYear( sYear, leaps );
	theTime.tm_isdst	=	-1;
	m_dTime = mktime(&theTime);

	m_dTime += LeapsToChTime_t(leaps);

}



ChTime::ChTime( const char* pstrUnivTime ) : m_dTime( 0 )
{
	ASSERT( pstrUnivTime );

	struct tm	theTime;

	ChMemClearStruct( &theTime );


	ChString strTime( pstrUnivTime );
	int iIndex = strTime.Find( TEXT( ',' ) );
	if ( iIndex != - 1 )
	{ // Remove the day string
		strTime = strTime.Right( strTime.GetLength() - iIndex - 1 );
		strTime.TrimLeft();
	}

	// get the date
	iIndex = strTime.Find( TEXT( ' ' ) );
	if ( iIndex == - 1 )
	{  // bogus string
		return;
	}
	ChString strDate;
	strDate = strTime.Left( iIndex );
	strTime = strTime.Right( strTime.GetLength() - iIndex - 1 );
	strTime.TrimLeft();

 	iIndex = strDate.Find( TEXT( '-' ) );

	if ( iIndex == - 1 )
	{  // bogus string
		return;
	}
	else
	{
		ChString strDay;
		strDay = strDate.Left( iIndex );
		strDay.TrimLeft();
		strDate = strDate.Right( strDate.GetLength() - iIndex - 1 );

	 	theTime.tm_mday		=	atol( strDay );
	}


	// get the month
	iIndex = strDate.Find( TEXT( '-' ) );
	if ( iIndex == - 1 )
	{  // bogus string
		return;
	}
	else
	{
		ChString strMonth;
		strMonth = strDate.Left( iIndex );
		strMonth.TrimLeft();
		strDate = strDate.Right( strDate.GetLength() - iIndex - 1 );

	 	theTime.tm_mon		= GetMonth( strMonth );
	}

	// get the year
	{
		strDate.TrimLeft();
		chint32		leaps;	// number of extra 4 year periods
		chint32		lYear = atol( strDate ) + 1900;
		if ( lYear < 1970 )
		{
			lYear = 1970;
		}
	 	theTime.tm_year		= NormalizeYear( (short)lYear, leaps );
	}


	// get the hour
	iIndex = strTime.Find( TEXT( ':' ) );
	if ( iIndex == - 1 )
	{  // bogus string
		return;
	}
	else
	{
		ChString strHour;
		strHour = strTime.Left( iIndex );
		strTime = strTime.Right( strTime.GetLength() - iIndex - 1 );
		strTime.TrimLeft();

	 	theTime.tm_hour		= atol( strHour );
	}

	// get the minute
	iIndex = strTime.Find( TEXT( ':' ) );
	if ( iIndex == - 1 )
	{  // bogus string	 ???  
		iIndex = strTime.Find( TEXT( ' ' ) );
		if ( iIndex == - 1 )
		{  // bogus string
			return;
		}
	 	theTime.tm_min	= atol( strTime );
	 	theTime.tm_sec	= 0;

	}
	else
	{
		ChString strMinute;
		strMinute = strTime.Left( iIndex );
		strTime = strTime.Right( strTime.GetLength() - iIndex - 1 );
		strTime.TrimLeft();

	 	theTime.tm_min	= atol( strMinute );
	 	theTime.tm_sec	= atol( strTime );
	}

	theTime.tm_isdst	=	0;

	m_dTime = mktime(&theTime);

}

int ChTime::GetMonth( const ChString& strMonth )
{
	for( int i = 0; i < 12; i++ )
	{
		if ( strMonth.CompareNoCase( pstrMonths[i] ) == 0 )
		{
			return i;
		}
	}
	return 0;
}


void ChTime::Serialize( ChArchive &archive )
{
						// First call base class

	ChStreamable::Serialize( archive );

						// Now serialize ourself

	if (modeWrite & archive.GetMode())
	{
						// Write out data

		chint32 lo32 = GetLo32(m_dTime);
		chint32 hi32 = GetHi32(m_dTime);
	
		archive << lo32;
		archive << hi32;
	}
	else
	{					// Read in data

		chint32 lo32;
		chint32 hi32;
		archive >> lo32;
		archive >> hi32;
		m_dTime = MakeChTime_t(lo32, hi32);;
	}
}


#if !defined( CH_ARCH_16 )
ChString ChTime::Format( const char *pstrFormat )
{
	TCHAR    strBuf[MAXTIMEBUFSIZE];
	#define DEFTIMEFORMAT	"%c"
	if (!pstrFormat) pstrFormat = DEFTIMEFORMAT;

	struct tm tms;
	if(GetLocTm( m_dTime, &tms ))
	{
	if (!strftime(strBuf, sizeof(strBuf) / sizeof(TCHAR), pstrFormat, &tms))
		strBuf[0] = '\0';
	}
	return ChString(strBuf);
}                        
#endif

//ChString ChTime::Format();

chint16 ChTime::GetDay() const
{
	struct tm tms;
	chint16 sDay = -1;
	 
	if(GetLocTm( m_dTime, &tms ))
	{
		sDay = tms.tm_mday;
	}
	return sDay;
}


chint16 ChTime::GetDayOfWeek() const
{

	struct tm tms;
	chint16 sDay = -1;
	 										
	if(GetLocTm( m_dTime, &tms ))
	{
		sDay = tms.tm_wday;
	}
	return sDay;
}

chint16 ChTime::GetHour() const
{

	struct tm tms;
	chint16 sHour = -1;
	 
	if(GetLocTm( m_dTime, &tms ))
	{
		sHour = tms.tm_hour;
	}
	return sHour;
}

chint16 ChTime::GetMinute() const
{
	struct tm tms;
	chint16 sMinute = -1;
	 
	if(GetLocTm( m_dTime, &tms ))
	{
		sMinute = tms.tm_min;
	}

	return sMinute;
}
chint16 ChTime::GetMonth() const
{
	struct tm tms;
	chint16 sMonth = -1;
	 
	if(GetLocTm( m_dTime, &tms ))
	{
		sMonth = tms.tm_mon;
	}
	return sMonth;

}
chint16 ChTime::GetSecond() const
{
	struct tm tms;
	chint16 sSecond = -1;
	 
	if(GetLocTm( m_dTime, &tms ))
	{
		sSecond = tms.tm_sec;
	}
	return sSecond;

}

chint16 ChTime::GetYear() const
{
	struct tm tms;
	chint16 sYear = -1;
	 
	if(GetLocTm( m_dTime, &tms ))
	{
		sYear = tms.tm_year;
	}
	return sYear + 1900;
}

struct tm* ChTime::GetLocalTm( struct tm* ptm /*= NULL */ )
{
	static struct tm time;    
	
	if ( ptm == NULL )
	{
		ptm = &time;
	}   
	
	GetLocTm( m_dTime, ptm );
	
	return ( ptm );
}


const ChTime& ChTime::operator =( const ChTime_t time )
{
	m_dTime = time;
	return *this;
}


ChTimeSpan ChTime::operator-(ChTime time) const
{ 
	return ChTimeSpan(m_dTime - time.m_dTime);
}

ChTime ChTime::operator-(ChTimeSpan timeSpan) const
{
	return ChTime(m_dTime - timeSpan.GetTotalSeconds());
}

ChTime ChTime::operator+(ChTimeSpan timeSpan) const
{
	return ChTime(m_dTime + timeSpan.GetTotalSeconds());
}

const ChTime& ChTime::operator+=(ChTimeSpan timeSpan)
{
	m_dTime += timeSpan.GetTotalSeconds();
	return *this;
}

const ChTime& ChTime::operator-=(ChTimeSpan timeSpan)
{
	m_dTime -= timeSpan.GetTotalSeconds();
	return *this;
}

bool ChTime::operator==(ChTime time) const
{ 
	return m_dTime == time.m_dTime;
}

bool ChTime::operator!=(ChTime time) const
{ 
	return m_dTime != time.m_dTime;
}

bool ChTime::operator<(ChTime time) const
{ 
	return m_dTime < time.m_dTime;
}

bool ChTime::operator>(ChTime time) const
{ 
	return m_dTime > time.m_dTime;
}

bool ChTime::operator<=(ChTime time) const
{ 
	return m_dTime <= time.m_dTime;
}

bool ChTime::operator>=(ChTime time) const
{ 
	return m_dTime >= time.m_dTime;
}


 /* static time methods */

ChTime ChTime::GetCurrentTime()
{
 	return ChTime(ChTime_t(::time(0)));
}

 /* Helper functions and methods */

static bool	GetLocTm( double dTime, struct tm *pTm )
{
	time_t	lTime;		// CRT's time
	chint32	leaps;	// number of extra 4 year periods
	
	lTime = NormalizeTime( dTime, leaps );

	// need a semaphore here
	struct tm* pTheTime = localtime(&lTime);
	if (pTheTime == NULL)
		return false;

	pTheTime->tm_year += leaps * 4;
	*pTm = *pTheTime;
	// end semaphore
	return true;

}
static inline ChTime_t LeapsToChTime_t(chint32 leaps)
{
	return ( TICKSPERLEAPPERIOD * leaps );	   // not right on nonleap century crossings
}

static inline chint16 NormalizeYear(chint16 sYear, chint32& leaps)
{
	leaps = 0;

	sYear -= 1900;	// we use all digits; tm wants 1900 based

	chint16 incr = (sYear < 70) ? -1 : 1;
	while (sYear < 70 || sYear > MAXTIMETYEAR)
	{
		leaps += incr; 
		sYear -= incr * 4;
	}
	return sYear;
}

static inline time_t NormalizeTime( double dTime, chint32 &leaps )
{
	leaps = 0;
	double dIncr;
	// could be faster, but why?
	chint16 incr = (dTime < 0) ? -1 : 1;
	dIncr =  TICKSPERLEAPPERIOD	* incr;

	while (dTime < 0 || dTime > MAXLONG)
	{
		leaps += incr; 
		dTime -= dIncr;
	}
	return (time_t(dTime));
}

static inline chint32 GetLo32(ChTime_t t)
{
	return chint32(floor(pmod(t, double(MAXLONG))));
}

static inline chint32 GetHi32(ChTime_t t)
{
	return chint32(floor(t / double(MAXLONG)));
}

static inline ChTime_t MakeChTime_t(chint32 lo, chint32 hi)
{
	return ((ChTime_t(hi) * MAXLONG) + lo);
}

static inline double pmod( double a, double div )
{
	double t = fmod(a, div);
	if (t < 0) t += div;
	return t;
}

/*----------------------------------------------------------------------------
	ChTimeSpan class
----------------------------------------------------------------------------*/

ChTimeSpan::ChTimeSpan( chint32 lDays, chint16 sHours, chint16 sMins, chint16 sSecs )
{
	m_span = lDays;
	m_span = m_span * 24 + sHours;
	m_span = m_span * 60 + sMins;
	m_span = m_span * 60 + sSecs;
}

void ChTimeSpan::Serialize( ChArchive &archive )
{
						// First call base class

	ChStreamable::Serialize( archive );

						// Now serialize ourself

	if (modeWrite & archive.GetMode())
	{
						// Write out data
		chint32 lo32 = GetLo32(m_span);
		chint32 hi32 = GetHi32(m_span);

		archive << lo32;
		archive << hi32;
	}
	else
	{					// Read in data

		chint32 lo32;
		chint32 hi32;
		archive >> lo32;
		archive >> hi32;
		m_span = MakeChTime_t(lo32, hi32);;
	}
}

#if !defined( CH_ARCH_16 )
ChString ChTimeSpan::Format( const char *pstrFormat)
{
	TCHAR strBuffer[MAXTIMEBUFSIZE];
	TCHAR ch;
	TCHAR* pch = strBuffer;
	#define DEFTIMESPANFORMAT	"%Dd:%Hh:%Mm:%Ss"
	if (!pstrFormat) pstrFormat = DEFTIMESPANFORMAT;

	while ((ch = *pstrFormat++) != '\0')
	{
		ASSERT(pch < &strBuffer[MAXTIMEBUFSIZE]);
		if (ch == '%')
		{
			switch (ch = *pstrFormat++)
			{
			case 'D':
				pch += sprintf(pch, "%ld", GetDays());
				break;
			case 'H':
				pch += sprintf(pch, "%02hd", GetHours());
				break;
			case 'M':
				pch += sprintf(pch, "%02hd", GetMinutes());
				break;
			case 'S':
				pch += sprintf(pch, "%02hd", GetSeconds());
				break;
			case '%':
				*pch++ = ch;
				break;
			default:
				//ASSERT(FALSE);
				break;
			}
		}
		else
		{
			*pch++ = ch;
			if (_istlead(ch))
			{
				ASSERT(pch < &strBuffer[MAXTIMEBUFSIZE]);
				*pch++ = *pstrFormat++;
			}
		}
	}

	*pch = '\0';
	return ChString(strBuffer);
}                
#endif


chint32 ChTimeSpan::GetDays() const
{
	return (chint32(floor(m_span / SECONDSPERDAY)));
}

chint16 ChTimeSpan::GetHours() const
{
	return chint16(floor(pmod(m_span, SECONDSPERDAY) / SECONDSPERHOUR));
}

chint16 ChTimeSpan::GetMinutes() const
{
	return chint16(floor(pmod(m_span, SECONDSPERHOUR) / SECONDSPERMINUTE));
}
chint16 ChTimeSpan::GetSeconds() const
{
	return chint16(floor(pmod(m_span, SECONDSPERMINUTE)));
}

chint32 ChTimeSpan::GetTotalHours() const
{
	return (chint32(floor(m_span / SECONDSPERHOUR)));
}

chint32 ChTimeSpan::GetTotalMinutes() const
{
	return (chint32(floor(m_span / SECONDSPERMINUTE)));
}


ChTimeSpan ChTimeSpan::operator +( ChTimeSpan span ) const
{
	return ChTimeSpan(m_span + span.m_span);
}

ChTimeSpan& ChTimeSpan::operator +=( ChTimeSpan span ) 
{
	m_span += span.m_span;
	return *this;
}

ChTimeSpan ChTimeSpan::operator -( ChTimeSpan span ) const
{
	return ChTimeSpan(m_span - span.m_span);
}

ChTimeSpan& ChTimeSpan::operator -=( ChTimeSpan span )
{
	m_span -= span.m_span;
	return *this;
}

ChTime ChTimeSpan::operator+(ChTime time) const
{
	return ChTime(time.GetTime() + m_span);
}

bool ChTimeSpan::operator==(ChTimeSpan span) const
{ 
	return m_span == span.m_span;
}

bool ChTimeSpan::operator!=(ChTimeSpan span) const
{ 
	return m_span != span.m_span;
}

bool ChTimeSpan::operator<(ChTimeSpan span) const
{ 
	return m_span < span.m_span;
}

bool ChTimeSpan::operator>(ChTimeSpan span) const
{ 
	return m_span > span.m_span;
}

bool ChTimeSpan::operator<=(ChTimeSpan span) const
{ 
	return m_span <= span.m_span;
}

bool ChTimeSpan::operator>=(ChTimeSpan span) const
{ 
	return m_span >= span.m_span;
}


// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
