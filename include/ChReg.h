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

	This file consists of the interface for the ChRegistry class, used to
	manipulate registry information on Microsoft Windows.

----------------------------------------------------------------------------*/

#if !defined( _CHREG_H )
#define _CHREG_H


#if defined( CH_MSW )
											/* Disable warnings about non-
												exported classes for MSW */
	#pragma warning( disable: 4275 )
	#pragma warning( disable: 4251 )

#endif	// defined( CH_MSW )


#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA    
#endif

#if defined( CH_UNIX )

	#include <ChDb.h>

	typedef void *HKEY;

	#define CH_REG_DB_NAME		"registry"

#endif


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

//#define CH_COMPANY_NAME		"Andromedia Incorporated"
#define CH_COMPANY_NAME		"Ultra Enterprises"

#if  defined( CH_PUEBLO_PLUGIN )
   //  Pueblo plugin
	#define	CH_PRODUCT_NAME	"Pueblo Plug-In"

#else
	// Pueblo standalone
	#define CH_PRODUCT_NAME		"Pueblo"

#endif
#define CH_VERSION_DEFAULT	""
#define CH_GROUP_PATH		"path"
#define CH_KEY_HELPFILES	"HelpFiles"

/*----------------------------------------------------------------------------
	ChRegistry class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChRegistry
{
	public:
		ChRegistry( const ChString& strCompanyName, const ChString& strProduct,
						const ChString& strGroup,
						const ChString& strVersion = CH_VERSION_DEFAULT );
		ChRegistry( const ChString& strGroup,
						const ChString& strVersion = CH_VERSION_DEFAULT );
		~ChRegistry();

		bool GetHelpPath( ChString& strHelpFiles );

		bool Erase( const ChString& strLabel );

		bool WriteBool( const ChString& strLabel, BOOL boolVal );
		bool Write( const ChString& strLabel, chint32 lVal );
		bool Write( const ChString& strLabel, const ChString& strVal );
		bool Read( const ChString& strLabel, chint32& lVal,
					chint32 lDefault = 0 );
		inline bool Read( const ChString& strLabel, chuint32& luVal,
							chuint32 luDefault = 0 )
				{
					chint32		lVal( luVal );
					bool		boolReturn;

					boolReturn = Read( strLabel, lVal, (chint32)luDefault );
					luVal = lVal;

					return boolReturn;
				}
		bool Read( const ChString& strLabel, chint16& sVal,
					chint16 sDefault = 0 );
		inline bool Read( const ChString& strLabel, chuint16& suVal,
							chuint16 suDefault = 0 )
				{
					chint16		sVal( suVal );
					bool		boolReturn;

					boolReturn = Read( strLabel, sVal, (chint16)suDefault );
					suVal = sVal;

					return boolReturn;
				}
		bool ReadBool( const ChString& strLabel, bool& boolVal,
						bool boolDefault );
		bool ReadBool( const ChString& strLabel, BOOL& boolVal,
						bool boolDefault ) {
			bool bTemp, bResult;
			bResult = ReadBool(strLabel, bTemp, boolDefault);
			boolVal = bTemp;
			return bResult;
		}
		bool Read( const ChString& strLabel, ChString& strVal,
						const ChString& strDefault = "" );

	protected:
		void Construct( const ChString& strCompanyName, const ChString& strProduct,
						const ChString& strGroup, const ChString& strVersion );

	private:  
		ChString		m_strCompanyName;
		ChString		m_strProduct;
		ChString		m_strGroup;
		ChString		m_strVersion;

		ChString		m_strMasterKey;
		HKEY		m_hMasterKey;

		#ifdef CH_UNIX
		static ChDataBase* m_pRegDB;
		static chuint32 m_luRefCount;
		void 	OpenDb(void);

		friend chint32 RegSetValue( HKEY hKey, const ChString& strLabel, chuint32 luFlags,
									char* pstrBuffer, chuint32 lLen );
		friend chint32 RegQueryValue( HKEY hKey, const ChString& strLabel, char* pstrBuffer,
									  chint32* pLen );
		#endif
};

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR    
#endif

#endif	// !defined( _CHREG_H )

// Local Variables: ***
// tab-width:4 ***
// End: ***
