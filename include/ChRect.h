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

	This file contains the definitions for Chaco rectangle class.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHRECT_H )
#define _CHRECT_H

#ifdef CH_UNIX

#include <stdlib.h>

class ChPoint
{
  public:
	ChPoint( int new_x, int new_y ) { x = new_x; y = new_y; };
	ChPoint() { x = y = 0; };
	
	chint32 x, y;
};

class ChRect
{
  public:
	ChRect(int new_left, int new_top, int new_right, int new_bottom ) 
	{
		left = new_left;
		top = new_top;
		right = new_right;
		bottom = new_bottom;
	};
	ChRect() { left = top = right = bottom = 0; };
	operator==( const ChRect& rtRect ) { 
		return ( (left == rtRect.left) && (top == rtRect.top) &&
				(right == rtRect.right) && (bottom == rtRect.bottom) );
	};
	chuint32 Width() const { return abs(right - left); };
	chuint32 Height() const { return abs(bottom - top); };
	bool IsRectEmpty() { return ((left == right) && (top == bottom)); };
	ChPoint TopLeft() { return (ChPoint(left, top)); };
	ChPoint BottomRight() { return (ChPoint(right, bottom)); };
	PtInRect( ChPoint& pt ) { 
		cerr << "ChRect::PtInRect()" << endl;
		return ( (pt.x >= left) && (pt.y <= right) &&
			(pt.y >= top) && (pt.y <= bottom) );
	}
	void InflateRect(chint32 width, chint32 height) {
		left -= width / 2;
		right += width / 2;
		top -= width / 2;
		bottom += width / 2;
	}

	chuint32 left, top, right, bottom;
};

#endif // CH_UNIX

#endif // _CHRECT_H

// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
