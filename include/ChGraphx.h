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

	Main header file for the ChTxtIn and ChTxtOut modules of the Pueblo
	system.  These handle simple text input and output, respectively.

----------------------------------------------------------------------------*/

// $Header$

#define CH_MODULE_GRAPHICS_ANIMATION		"Chaco Graphics Animation module"
#define CH_MODULE_GRAPHICS_MAZE				"Chaco VRML Module"
#define CH_MODULE_GRAPHICS_PANE				"Chaco Graphics Pane Manager Module"
#define CH_MODULE_GRAPHICS_BASE				"ChGraphx"

#if !defined( _CHGRAPHX_H )
#define _CHGRAPHX_H

#ifdef CH_UNIX
#include <ChRect.h>
#endif

#if defined( CH_CLIENT ) || defined( CH_VRML_VIEWER )

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif

#if !defined(CH_VRML_PLUGIN )

#include <ChModule.h>
//#include <ChModMgr.h>
#include <ChCore.h>
#include <ChMsg.h>
#include <ChMsgTyp.h>

#endif	

#include<ChList.h>

/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define CH_CURSOR_STD_PTR	0x01
#define CH_CURSOR_HAIRS		0x02
#define CH_CURSOR_WAIT		0x03
#define CH_CURSOR_PICK		0x04
#define CH_CURSOR_IBEAM		0x05
#define CH_CURSOR_NO		0x06


/*----------------------------------------------------------------------------
	Message constants
----------------------------------------------------------------------------*/

#define CH_GR_MSG_MOVE_CAMERA	(CH_MSG_USER + 1)


/*----------------------------------------------------------------------------
	Type definitions
----------------------------------------------------------------------------*/

#endif

#if defined( CH_CLIENT ) && !defined(CH_VRML_PLUGIN )

/*----------------------------------------------------------------------------
	Classes
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChPoint3
{
	public:
		chint32	x;
		chint32	y;
		chint32	z;

	public:

// Constructors
	ChPoint3() :
		x(0), y(0), z(0) {};

	ChPoint3(chint32 initX, chint32 initY, chint32 initZ) :
		x(initX), y(initY), z(initZ) {};

// Operations
	void Offset(chint32 xOffset, chint32 yOffset, chint32 zOffset)
		{
			x += xOffset;
			y += yOffset;
			z += zOffset;
		};
	void Offset(const ChPoint3& point)
		{
			x += point.x;
			y += point.y;
			z += point.z;
		};
	BOOL operator==(ChPoint3 point) const
		{
			return (x==point.x && y==point.y && z==point.z);
		};
	BOOL operator!=(ChPoint3 point) const
		{
			return (x!=point.x || y!=point.y || z!=point.z);
		};

	void operator+=(ChPoint3 point)
		{
			x += point.x;
			y += point.y;
			z += point.z;
		};

	void operator-=(ChPoint3 point)
 		{
			x -= point.x;
			y -= point.y;
			z -= point.z;
		};

	ChPoint3 operator+(ChPoint3 point) const
 		{
			chint32	ix, iy, iz;
			ix = x + point.x;
			iy = y + point.y;
			iz = z + point.z;
			return ChPoint3(ix, iy, iz);
		};


	ChPoint3 operator-(ChPoint3 point) const
 		{
			chint32	ix, iy, iz;
			ix = x - point.x;
			iy = y - point.y;
			iz = z - point.z;
			return ChPoint3(ix, iy, iz);
		};
	ChPoint3 operator-() const
 		{
			return ChPoint3(-x, -y, -z);
		};

};
 
class ChCell {

	public:

		ChCell(chint32 ix = 0, chint32 iy = 0, chint32 iz = 0, 
			chint32 isprite = 0, chint32 irow = 0, chint32 icol = 0):
				x(ix), y(iy), z(iz), sprite(isprite), row(irow), col(icol) { };

		void Serialize( ChArchive& ar ) 
			{
				if( ar.GetMode() == modeRead )
				{
				    ar >> x;
				    ar >> y;
				    ar >> z;
					ar >> sprite;
				    ar >> row;
				    ar >> col;
				}
				else
				{
				    ar << x;
				    ar << y;
				    ar << z;
					ar << sprite;
				    ar << row;
				    ar << col;
				}
			}
	
	    chint32 x;
	    chint32 y;
	    chint32 z;
		chint32	sprite;	// sprite id
	    chint32 row;
	    chint32 col;
	    //CWave* pSnd;   // needs to be id ?
}; 

class ChAnimScript {

	public:

		ChAnimScript():
			m_iCurCell(0), m_iNumCells(0), m_lFrameRate(18), m_pCells(0), m_boolSave(false) { };
		virtual ~ChAnimScript()
			{ 
				if (!m_boolSave)
				{
					delete [] m_pCells;
				}
			};

		void Serialize( ChArchive& ar ) 
			{
				if( ar.GetMode() == modeRead )
				{
				    ar >> m_iNumCells;
				    ar >> m_iCurCell;
					ar >> m_lFrameRate;
				    m_pCells = new ChCell [m_iNumCells];
				}
				else
				{
				    ar << m_iNumCells;
				    ar << m_iCurCell;
					ar << m_lFrameRate;
				}
				for (int j = 0; j < m_iNumCells; j++)
				{
					m_pCells[j++].Serialize(ar);
				}
			};
		inline chint32 GetCurCell() { return m_iCurCell;}
		inline chint32 GetCount() { return m_iNumCells;}
		inline chint32 GetFrameRate() { return m_lFrameRate;}
		inline ChCell * GetCells() { return m_pCells;}
		inline void SetCurCell(chint32 iCurCell) {  m_iCurCell = iCurCell;}
		inline void SetCount(chint32 iNumCells) {  m_iNumCells = iNumCells;}
		inline void SetFrameRate(chint32 lFrameRate) {  m_lFrameRate = lFrameRate;}
		inline void SetCells(ChCell * pCells) {  m_pCells = pCells;}
		inline void SaveCells(bool boolSave = true) {  m_boolSave = boolSave;}

		chint32 m_iCurCell;
		chint32 m_iNumCells;
		chint32 m_lFrameRate;
		ChCell *m_pCells;
		bool	m_boolSave;
	
    //CWave* pSnd;   // needs to be id ?
}; 


/*----------------------------------------------------------------------------
	ChSceneMsg class - Load a scene, or append/modify one - sent by pane mgr
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChSceneMsg : public ChMsg
{
	public:
		ChSceneMsg( const ChString& strURL, const ChString& strFile, const ChString & strType, bool boolNew = true,  chparam data = 0 ) :
			ChMsg( CH_MSG_LOAD_SCENE )
				{
					ChArchive	archive( this, modeWrite );

					SetParam1( boolNew );		// whether to flush old scene
					archive << strURL;
					archive << strFile;
					archive << strType;
					archive << data;
				};


		inline void GetParams( ChString& strURL, ChString& strFile, ChString & strType, bool& boolNew )
				{
					ChArchive	archive( this, modeRead );
					boolNew = (GetParam1() != FALSE);
					archive >> strURL;
					archive >> strFile;
					archive >> strType;
				};
		inline void GetParams( ChString& strURL, ChString& strFile, ChString & strType, bool& boolNew, chparam &data )
				{
					ChArchive	archive( this, modeRead );
					boolNew = (GetParam1() != FALSE);
					archive >> strURL;
					archive >> strFile;
					archive >> strType;
					archive >> data;
				};
};


/*----------------------------------------------------------------------------
	ChCastMsg class	-- OBSOLETE except for delete
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChCastMsg : public ChMsg
{
	public:
		enum type { direct = 1, url, remove };
	public:
	#if 0
		ChCastMsg( chuint32 luId, const ChString& strSpriteURL,
			chint32 iNumSpriteRows = 1, chint32 iNumSpriteCols = 1, 
			chint32 iNumCells = 1, ChCell *pCells = 0, chint32 iCurCell = 0,
			chint32 lFrameRate = 30,
			bool boolVisible = true, 
			bool	boolFrozen = false, 
			bool boolDraggable = false) : ChMsg( CH_MSG_LOAD_CAST )
				{
					ChCell defCell;

					ChArchive	archive( this, modeWrite );
					SetParam1( direct );		// version
					archive << luId;
					archive << boolVisible;
					archive << boolFrozen;
					archive << boolDraggable;
					archive << strSpriteURL;
					archive << iNumSpriteRows;
					archive << iNumSpriteCols;
					archive << iNumCells;
					archive << iCurCell;
					archive << lFrameRate;
					if (!pCells)
					{
						pCells = &defCell;
					}
					for (int j = 0; j < iNumCells; j++)
					{
						pCells[j].Serialize(archive);
					}
				};

		ChCastMsg( const ChString& strCastURL ) : ChMsg( CH_MSG_LOAD_CAST )
				{
					ChArchive	archive( this, modeWrite );
					SetParam1( url );		// version
					archive << strCastURL;
				};
	#endif
		ChCastMsg( chuint32 luId ) : ChMsg( CH_MSG_LOAD_CAST )	// remove cast
				{
					ChArchive	archive( this, modeWrite );
					SetParam1( remove );		// version
					archive << luId;
				};
	#if 0
		// direct only
		inline void GetParams( chuint32& luId, ChString& strSpriteURL, 
			chint32& iNumSpriteRows, chint32& iNumSpriteCols, 
			chint32& iNumCells, ChCell *pCells, chint32& iCurCell,
			chint32& lFrameRate,
			bool& boolVisible, 
			bool& boolFrozen, 
			bool& boolDraggable )
				{
					ChArchive	archive( this, modeRead );

					archive >> luId;
					archive >> boolVisible;
					archive >> boolFrozen;
					archive >> boolDraggable;
					archive >> strSpriteURL;
					archive >> iNumSpriteRows;
					archive >> iNumSpriteCols;
					archive >> iNumCells;
					archive >> iCurCell;
					archive >> lFrameRate;
					if (pCells)
					{
						for (int j = 0; j < iNumCells; j++)
						{
							pCells->Serialize(archive);
							pCells ++;
						}
					}

				};
		inline void GetParams(  ChString &strURL )	// indirect only
		{
					// should assert on type == url, or something nicer
					ChArchive	archive( this, modeRead );

					archive >> strURL;
		}

	#endif
		inline void GetParams( chuint32& luId )	  // 'remove' type
				{
					ChArchive	archive( this, modeRead );

					archive >> luId;

				};
		inline type GetType()
			{
				return ((enum type)(GetParam1()));
			}
};


/*----------------------------------------------------------------------------
	ChAnchorMsg class -- OBSOLETE??
----------------------------------------------------------------------------*/
#ifndef CH_UNIX
class CH_EXPORT_CLASS ChAnchorMsg : public ChMsg
{
	public:
			   
		enum type { direct_rect = 1, direct_cast, url, remove };
	public:
		ChAnchorMsg( chuint32 id, ChRect& rcHot, ChString& strCmd, 
			ChString& strHint, chuint32 luCursor = CH_CURSOR_PICK ) :
					ChMsg( CH_MSG_LOAD_ANCHOR )
				{
					chuint32 luHotCastId = 0;
					SetParam1(direct_rect);	// version (or subtype?)
					ChArchive	archive( this, modeWrite );
					archive << id;
					archive << luHotCastId;
					archive << (chint32)rcHot.TopLeft().x;
					archive << (chint32)rcHot.TopLeft().y;
					archive << (chint32)rcHot.BottomRight().x;
					archive << (chint32)rcHot.BottomRight().y;
					archive << strCmd;
					archive << luCursor;
	 				archive << strHint;
					// later followed by character id, cell, etc. 
				}

		ChAnchorMsg( chuint32 id, chuint32 luHotCastId, ChString& strCmd, 
			ChString& strHint, chuint32 luCursor = CH_CURSOR_PICK ) :
					ChMsg( CH_MSG_LOAD_ANCHOR )
				{
					ChRect rcHot(0,0,0,0);
					SetParam1(direct_cast);	// version (or subtype?)
					ChArchive	archive( this, modeWrite );
					archive << id;
					archive << luHotCastId;
					archive << (chint32)rcHot.TopLeft().x;
					archive << (chint32)rcHot.TopLeft().y;
					archive << (chint32)rcHot.BottomRight().x;
					archive << (chint32)rcHot.BottomRight().y;
					archive << strCmd;
					archive << luCursor;
	 				archive << strHint;
					// later followed by character id, cell, etc. 
				}

 		ChAnchorMsg( ChString &strURL ) :
					ChMsg( CH_MSG_LOAD_ANCHOR )
		{
					// should assert on type, or something nicer
					SetParam1(url);	// version (or subtype?)
					ChArchive	archive( this, modeWrite );

					archive << strURL;
		}


 		ChAnchorMsg( chuint32 id ) :
					ChMsg( CH_MSG_LOAD_ANCHOR )
		{
					// should assert on type, or something nicer
					SetParam1(remove);	// version (or subtype?)
					ChArchive	archive( this, modeWrite );

					archive << id;
		}


		inline void GetParams( chuint32& id, ChRect& rcHot, ChString& strCmd, 
			ChString& strHint, chuint32& luCursor )
				{
					// should assert on type, or something nicer
					chuint32 luHotCastId = 0;
					ChArchive	archive( this, modeRead );

					archive >> id;
					archive >> luHotCastId; 
					#if defined( CH_ARCH_16 )
					archive >> (int)rcHot.TopLeft().x;
					archive >> (int)rcHot.TopLeft().y;
					archive >> (int)rcHot.BottomRight().x;
					archive >> (int)rcHot.BottomRight().y;   
					#else
					archive >> (chint32)rcHot.TopLeft().x;
					archive >> (chint32)rcHot.TopLeft().y;
					archive >> (chint32)rcHot.BottomRight().x;
					archive >> (chint32)rcHot.BottomRight().y;   
					#endif
					archive >> strCmd;
					archive >> luCursor;
					archive >> strHint;
				};

		inline void GetParams( chuint32& id, chuint32& luHotCastId, ChString& strCmd, 
			ChString& strHint, chuint32& luCursor )
				{
					// should assert on type, or something nicer
					ChRect rcHot(0,0,0,0);
					ChArchive	archive( this, modeRead );

					archive >> id;
					archive >> luHotCastId;
					#if defined( CH_ARCH_16 )
					archive >> (int)rcHot.TopLeft().x;
					archive >> (int)rcHot.TopLeft().y;
					archive >> (int)rcHot.BottomRight().x;
					archive >> (int)rcHot.BottomRight().y; 
					#else
					archive >> (chint32)rcHot.TopLeft().x;
					archive >> (chint32)rcHot.TopLeft().y;
					archive >> (chint32)rcHot.BottomRight().x;
					archive >> (chint32)rcHot.BottomRight().y; 
					#endif
					archive >> strCmd;
					archive >> luCursor;
					archive >> strHint;
				};


		inline void GetParams( ChString &strURL )
		{
					// should assert on type, or something nicer
					ChArchive	archive( this, modeRead );

					archive >> strURL;
		}

		inline void GetParams( chuint32& id )	 // remove form of message
		{
					// should assert on type, or something nicer
					ChArchive	archive( this, modeRead );

					archive >> id;
		}

		inline type GetType()
			{
				return ((enum type)(GetParam1()));
			}
};
#endif

/*----------------------------------------------------------------------------
	ChScriptMsg class	OBSOLETE??
----------------------------------------------------------------------------*/
#if 0
class CH_EXPORT_CLASS ChScriptMsg : public ChMsg
{
	public:
		ChScriptMsg( chuint32 luId, int iNumCells = 1, 
			ChCell *pCells = 0, int iCurCell = 0) : ChMsg( CH_MSG_LOAD_CAST )
				{
					ChCell defCell;

					ChArchive	archive( this, modeWrite );
					SetParam1( 1 );		// version
					archive << luId;
					archive << iNumCells;
					archive << iCurCell;
					if (!pCells)
					{
						pCells = &defCell;
					}
					for (int j = 0; j < iNumCells; j++)
					{
						pCells[j++].Serialize(archive);
					}
				};


		inline void GetParams( chuint32& luId,  
			int& iNumCells, ChCell *pCells, int& iCurCell )
				{
					ChArchive	archive( this, modeRead );

					archive >> luId;
					archive >> iNumCells;
					archive >> iCurCell;
					if (pCells)
					{
						for (int j = 0; j < iNumCells; j++)
						{
							pCells->Serialize(archive);
							pCells ++;
						}
					}

				};
};
#endif


/*----------------------------------------------------------------------------
	ChPlayGraphicMsg class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChPlayGraphicMsg : public ChMsg
{
	public:
		enum type {on_load_complete, immediate};
		
	public:
		ChPlayGraphicMsg( chuint32 luId = 0, bool boolPlay = true, type when = on_load_complete) :
			ChMsg( CH_MSG_PLAY_GRAPHIC )
				{
					SetParam1( 1 );		// version
					SetParam2( boolPlay );
					ChArchive	archive( this, modeWrite );

					archive << luId;
					archive << (chint32)when;
				};


		inline void GetParams( chuint32& luId, bool& boolPlay, type& when )
				{
					boolPlay = (GetParam2() != FALSE);
					ChArchive	archive( this, modeRead );
					chint32 temp;

					archive >> luId;
					archive >> temp;
					when = (type)temp;
				};
};


/*----------------------------------------------------------------------------
	ChShowCastMsg class 
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChShowCastMsg : public ChMsg
{
	public:
		ChShowCastMsg( chuint32 luId , bool boolShow = true ) :
			ChMsg( CH_MSG_SHOW_CAST )
				{
					SetParam1( 1 );		// version
					SetParam2( boolShow );
					ChArchive	archive( this, modeWrite );

					archive << luId;
				};


		inline void GetParams( chuint32& luId, bool& boolShow )
				{
					boolShow = (GetParam2() != FALSE);
					ChArchive	archive( this, modeRead );

					archive >> luId;
				};
};


/*----------------------------------------------------------------------------
	ChEnableDragMsg class -- not implemented yet
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChEnableDragMsg : public ChMsg
{
	public:

		ChEnableDragMsg( const ChString& strURL ) :
			ChMsg( CH_MSG_ENABLE_DRAG )
				{
					ChArchive	archive( this, modeWrite );

					archive << strURL;
				};


		inline void GetParams( ChString& strURL )
				{
					ChArchive	archive( this, modeRead );

					archive >> strURL;
				};
};


/*----------------------------------------------------------------------------
	ChMoveCameraMsg class -- move the camera position in 3D space
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChMoveCameraMsg : public ChMsg
{
	public:

		ChMoveCameraMsg( float fPosX, float fPosY, float fPosZ,
							float fOrientationAxisX, float fOrientationAxisY,
							float fOrientationAxisZ, float fOrientationAngle ) :
			ChMsg( CH_GR_MSG_MOVE_CAMERA )
				{
					ChArchive	archive( this, modeWrite );

					archive << fPosX << fPosY << fPosZ <<
								fOrientationAxisX << fOrientationAxisY <<
								fOrientationAxisZ << fOrientationAngle;
				};


		inline void GetParams( float& fPosX, float& fPosY, float& fPosZ,
								float& fOrientationAxisX,
								float& fOrientationAxisY,
								float& fOrientationAxisZ,
								float& fOrientationAngle )
				{
					ChArchive	archive( this, modeRead );

					archive >> fPosX >> fPosY >> fPosZ >>
								fOrientationAxisX >> fOrientationAxisY >>
								fOrientationAxisZ >> fOrientationAngle;
				};
};


#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	#undef AFXAPP_DATA
	#define AFXAPP_DATA NEAR    
#endif

#endif	//  defined( CH_CLIENT ) && !defined(CH_VRML_PLUGIN )
#endif	// !defined( _CHGRAPHX_H )

// $Log$
