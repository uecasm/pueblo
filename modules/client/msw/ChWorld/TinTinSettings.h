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

	TinTin class definitions.  Originally modified from TinTin++,
	(T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t, originally coded by
	Peter Unold 1992.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( TINTINSETTINGS_H )
#define TINTINSETTINGS_H

#include "TinTinList.h"


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define DEF_TINTIN_CHAR			'#'			// TinTin escape char
#define DEF_VERBATIM_CHAR		'\\'		/* If an input starts with this
												char, it will be sent 'as is'
												to the MUD */
#define DEF_TICK_SIZE			75
#define DEF_TICK_KEY			"TinTin"


/*----------------------------------------------------------------------------
	Type definitions
----------------------------------------------------------------------------*/

enum { msgIndexActions, msgIndexAliases, msgIndexAntiSub,
		msgIndexHilight, msgIndexPathdir, msgIndexSubstitutions,
		msgIndexVariable, msgIndexLast };


/*----------------------------------------------------------------------------
	TinTinSettings class
----------------------------------------------------------------------------*/

class TinTinSettings
{
	public:
		TinTinSettings();
		TinTinSettings( TinTinSettings& src );
		~TinTinSettings();

		inline const ChString& GetTickKey() { return m_strTickKey; }
		inline char GetTinTinChar() const { return m_cTinTin; }

		inline TinTinList* GetListActions() const { return m_pListActions; }
		inline TinTinList* GetListAliases() const { return m_pListAliases; }
		inline TinTinList* GetListVars() const { return m_pListVars; }
		inline TinTinList* GetListHighlights() const
				{
					return m_pListHighlights;
				}
		inline TinTinList* GetListSubs() const { return m_pListSubs; }
		inline TinTinList* GetListAntiSubs() const { return m_pListAntiSubs; }
		inline TinTinList* GetListPathDirs() const { return m_pListPathDirs; }
		inline int GetTickSize() const { return m_iTickSize; }

		inline bool IsDisplayingMsg( int iClass ) const
				{
					return m_boolDisplayMessages[iClass];
				}
		inline bool IsEcho() const { return m_boolEcho; }
		inline bool IsIgnore() const { return m_boolIgnore; }
		inline bool IsSpeedwalk() const { return m_boolSpeedwalk; }
		inline bool IsTicking() const { return m_boolTicking; }
		inline bool IsUsingPresubs() const { return m_boolPresub; }
		inline bool IsUsingSubs() const { return m_boolUsingSubs; }
		inline bool IsVerbatim() const { return m_boolVerbatim; }

		inline void SetDisplayingMsg( int iClass, bool boolDisplay )
						{
							m_boolDisplayMessages[iClass] = boolDisplay;
						}
		inline void SetEcho( bool boolEcho )
						{
							m_boolEcho = boolEcho;
						}
		inline void SetIgnore( bool boolIgnore ) { m_boolIgnore = boolIgnore; }
		inline void SetPresub( bool boolPresub )
						{
							m_boolPresub = boolPresub;
						}
		inline void SetSpeedwalk( bool boolSpeedwalk )
						{
							m_boolSpeedwalk = boolSpeedwalk;
						}
		inline void SetTicking( bool boolTicking )
						{
							m_boolTicking = boolTicking;
						}
		inline void SetTickKey( const ChString& strKey )
						{
							m_strTickKey = strKey;
						}
		inline void SetTickSize( int iSize )
						{
							m_iTickSize = iSize;
						}
		inline void SetTinTinChar( char cNewTinTin )
						{
							m_cTinTin = cNewTinTin;
						}
		inline void SetUsingSubs( bool boolUsingSubs )
						{
							m_boolUsingSubs = boolUsingSubs;
						}
		inline void SetVerbatim( bool boolVerbatim )
						{
							m_boolVerbatim = boolVerbatim;
						}

		TinTinSettings& operator=( TinTinSettings& src );

		void Reset();
		void DoKillAll();

	protected:
		void Initialize();

	protected:
		TinTinList*			m_pListAliases;
		TinTinList*			m_pListActions;
		TinTinList*			m_pListVars;
		TinTinList*			m_pListHighlights;
		TinTinList*			m_pListSubs;
		TinTinList*			m_pListAntiSubs;
		TinTinList*			m_pListPathDirs;

		char				m_cTinTin;

		bool				m_boolDisplayMessages[msgIndexLast];

		bool				m_boolEcho;
		bool				m_boolIgnore;
		bool				m_boolPresub;
		bool				m_boolSpeedwalk;
		bool				m_boolUsingSubs;
		bool				m_boolVerbatim;

		bool				m_boolTicking;
		int					m_iTickSize;	// in seconds
		ChString				m_strTickKey;
};


#endif	// !defined( TINTINSETTINGS_H )

// $Log$
