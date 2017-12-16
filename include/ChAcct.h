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

	This file consists of the interface for the ChAccountInfo class.

----------------------------------------------------------------------------*/

#if (!defined( _CHACCT_H ))
#define _CHACCT_H        

#include <ChTypes.h>
#include <ChData.h>
#include <ChStrmbl.h>    

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif


/*----------------------------------------------------------------------------
	ChAccountInfo class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChAccountInfo : public ChStreamable
{
	public:
		typedef enum tagGender { male = 1, female, other, noyb } Gender;

	public:
		ChAccountInfo();
		~ChAccountInfo();

		inline const ChString& GetFirstName() { return m_strFirstName; }
		inline const ChString& GetMiddleName() { return m_strMiddleName; }
		inline const ChString& GetSurname() { return m_strSurname; }
		inline const ChString& GetStreet() { return m_strStreet; }
		inline const ChString& GetApartment() { return m_strApt; }
		inline const ChString& GetCity() { return m_strCity; }
		inline const ChString& GetState() { return m_strState; }
		inline const ChString& GetZip() { return m_strZip; }
		inline const ChString& GetCountry() { return m_strCountry; }
		inline const ChString& GetEMail() { return m_strEMail; }
		inline Gender GetGender() { return m_gender; }
		inline const ChString& GetMiscFields() { return m_strMisc; }

		inline void SetFirstName( const ChString& strFirstName )
							{ m_strFirstName = strFirstName; }
		inline void SetMiddleName( const ChString& strMiddleName )
							{ m_strMiddleName = strMiddleName; }
		inline void SetSurname( const ChString& strSurname )
							{ m_strSurname = strSurname; }
		inline void SetStreet( const ChString& strStreet )
							{ m_strStreet = strStreet; }
		inline void SetApartment( const ChString& strApartment )
							{ m_strApt = strApartment; }
		inline void SetCity( const ChString& strCity ) { m_strCity = strCity; }
		inline void SetState( const ChString& strState )
							{ m_strState = strState; }
		inline void SetZip( const ChString& strZip ) { m_strZip = strZip; }
		inline void SetCountry( const ChString& strCountry )
							{ m_strCountry = strCountry; }
		inline void SetEMail( const ChString& strEMail ) { m_strEMail = strEMail; }
		inline void SetGender( Gender gender ) { m_gender = gender; }

		void SetName( const ChString& strFirstName, const ChString& strMiddleName,
						const ChString& strSurname );
		void SetAddress( const ChString& strStreet, const ChString& strApartment,
							const ChString& strCity, const ChString& strState,
							const ChString& strZip, const ChString& strCountry,
							const ChString& strEMail );
		inline void SetPersonal( Gender gender )
							{
								SetGender( gender );
							}

		void AddMiscField( const ChString& strLabel, const ChString& strValue );

											// Overrides
		virtual void Serialize( ChArchive& ar );

	private:
		ChString		m_strFirstName;
		ChString		m_strMiddleName;
		ChString		m_strSurname;			// Last or family name
		
		ChString		m_strStreet;
		ChString		m_strApt;
		ChString		m_strCity;
		ChString		m_strState;				// or Province
		ChString		m_strZip;
		ChString		m_strCountry;

		ChString		m_strEMail;

		Gender		m_gender;

		ChString		m_strMisc;
}; 

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR 
#endif

#endif	// !defined( _CHACCT_H )
