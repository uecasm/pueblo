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

	Utility functions for the Sound module.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"

#include "ChSoundUtils.h"
#include "MemDebug.h"


/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/

#define SLIDER_TICKS				8
#define SLIDER_PAGES				4
#define SLIDER_LINES				16
#define SLIDER_RANGE_MULTIPLIER		0xFFF

const int	suVolumeRange = VOLUME_MAX_RANGE / SLIDER_RANGE_MULTIPLIER;


/*----------------------------------------------------------------------------
	Functions
----------------------------------------------------------------------------*/

chuint16 GetSliderVolume( CSliderCtrl* pSlider )
{
	chint32		lVolume;
	int			iPos;

	iPos = pSlider->GetPos();
	iPos = InvertVolumeSliderPos( (chuint16)iPos );
	lVolume = iPos * SLIDER_RANGE_MULTIPLIER;

	ASSERT( lVolume <= VOLUME_MAX_RANGE );

	return (chuint16)lVolume;
}


int InvertVolumeSliderPos( int iPos )
{
	iPos -= suVolumeRange;
	iPos = abs( iPos );
											// Range check
	if (iPos > suVolumeRange)
	{
		iPos = suVolumeRange;
	}

	return iPos;
}


void InitVolumeSlider( CSliderCtrl& slider )
{
	slider.SetRange( 0, suVolumeRange );
	slider.SetLineSize( suVolumeRange / SLIDER_LINES );
	slider.SetPageSize( suVolumeRange / SLIDER_PAGES );
	slider.SetTicFreq( suVolumeRange / SLIDER_TICKS );
}


void ReadVolumeSliderPos( CSliderCtrl& slider, ChRegistry& reg, const char* strRegKey )
{
	chint32		lVolume;
	chuint16	suPos;

	reg.Read( strRegKey, lVolume, VOLUME_MAX_RANGE );
	if (lVolume > VOLUME_MAX_RANGE)
	{
		lVolume = VOLUME_MAX_RANGE;
	}
	suPos = (chuint16)(lVolume / SLIDER_RANGE_MULTIPLIER);

	ASSERT( suPos <= suVolumeRange );

	suPos = InvertVolumeSliderPos( suPos );
	slider.SetPos( suPos );
}


void WriteVolumeSliderPos( CSliderCtrl& slider, ChRegistry& reg, const char* strRegKey )
{
	chint32		lVolume;
	int			iPos;

	iPos = slider.GetPos();
	iPos = InvertVolumeSliderPos( (chuint16)iPos );
	lVolume = iPos * SLIDER_RANGE_MULTIPLIER;

	ASSERT( lVolume <= VOLUME_MAX_RANGE );

	reg.Write( strRegKey, lVolume );
}

// $Log$
// Revision 1.1.1.1  2003/02/03 18:53:03  uecasm
// Import of source tree as at version 2.53 release.
//
