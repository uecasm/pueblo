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

	This file all constants and tydefn of tables used by HTML parser.

----------------------------------------------------------------------------*/

// $Header$

/*----------------------------------------------------------------------------
	Constants
----------------------------------------------------------------------------*/


#if (!defined( _CHHTMSYM_H ))
#define _CHHTMSYM_H

#define	CHAR_BULLET				0
#define CHAR_SQUARE				1



// !!!!!!!!
// These constants are  indexes to pstrHTMLTokens array Do not modify these constants.
//  Add costants to the end and update the 	pstrHTMLTokens to  add new tokens
#define  HTML_UNKNOWN		0

#define	 HTML_H1			1
#define	 HTML_H2			2
#define	 HTML_H3			3
#define	 HTML_H4			4
#define	 HTML_H5			5
#define  HTML_H6			6

#define  HTML_BLKQUOTE		7
#define  HTML_ADDRESS		8
#define  HTML_PARAGRAPH		9
#define  HTML_COMMENT		10
#define  HTML_PREFORMAT		11

#define  HTML_BULLETLINE	12
#define  HTML_NUMLIST		13
#define  HTML_DEFNLIST		14
#define	 HTML_DIR			15

#define  HTML_MENU			16

#define  HTML_QUOTE			17

#define  HTML_STRIKETHROUGH	18
#define  HTML_BOLD			19
#define  HTML_ITALIC		20
#define  HTML_UNDERLINE		21

#define  HTML_TYPEWRITER	22

#define  HTML_EMPHASIS		23
#define  HTML_CITE			24
#define  HTML_STRONG		25

#define  HTML_CODE			26	

#define  HTML_SAMP			27

#define  HTML_HORZRULE		28

#define  HTML_LINEBREAK		29

#define  HTML_LINEITEM		30

#define  HTML_LINK			31

#define	 HTML_DEFNTEXT		32
#define  HTML_DEFNTERM		33

#define  HTML_CENTER		34

#define  HTML_HTML			35
#define  HTML_HEAD			36
#define  HTML_BODY			37
#define  HTML_TITLE			38
#define  HTML_PAGE			39
#define  HTML_PREFETCH		40
#define  HTML_IMAGE			41

#define  HTML_BASEFONT		42
#define  HTML_FONT			43

#define  HTML_FORM			44
#define  HTML_INPUT			45
#define  HTML_OPTION		46
#define  HTML_SELECT		47
#define  HTML_TEXTAREA		48

#define  HTML_MUDTEXT		49   
 
#define  HTML_XMP			50

#define  HTML_LISTING		51
#define  HTML_PLAINTEXT		52

#define  HTML_NOBR			53

#define  HTML_BASE			54
#define  HTML_XCHPANE		55

#define HTML_EMBED			56
//#define  MAX_TAGS			57




// Arguments we process
#define  ARG_ALIGN				1
#define  ARG_SIZE				2
#define  ARG_WIDTH				3
#define  ARG_HEIGHT				4
#define  ARG_HREF				5
#define  ARG_SRC				6
#define  ARG_CLEAR				7
#define  ARG_XCMD				8
#define  ARG_TYPE				9
#define  ARG_TXTCOLOR			10
#define  ARG_FGCOLOR			11
#define  ARG_BGCOLOR			12
#define  ARG_LINK				13
#define  ARG_VLINK				14
#define	 ARG_ACTION 			15
#define	 ARG_METHOD 			16
#define  ARG_ENCTYPE 			17
#define	 ARG_DISABLED 			18
#define	 ARG_SELECTED 			19
#define	 ARG_VALUE 				20
#define	 ARG_CHECKED 			21
#define	 ARG_MAXLENGTH 			22
#define	 ARG_NAME 				23
#define	 ARG_MULTIPLE 			25
#define	 ARG_ROWS 				26
#define	 ARG_COLS 				27
#define  ARG_START				28
#define  ARG_XWORLD				29
#define  ARG_XPROBABLITY		30
#define  ARG_ALINK				31
#define  ARG_PLINK				32
#define  ARG_HSPACE				33
#define  ARG_VSPACE				34
#define  ARG_ISMAP				35
#define  ARG_BACKGROUND			36
#define  ARG_BORDER				37
#define  ARG_NOSHADE			38
#define  ARG_NOWRAP				39
#define  ARG_TARGET				40
#define  ARG_PANETITLE			41
#define  ARG_MINWIDTH			42
#define  ARG_MINHEIGHT			43
#define  ARG_OPTIONS			44
#define  ARG_ALIGNTO			45
#define  ARG_SCROLLING			46
#define  ARG_XGRAPH				47 
#define  ARG_MD5				48
#define  ARG_COLOR				49


#define	 VAL_NUM	 			0
#define	 VAL_CHARUPPER			1
#define	 VAL_CHARLOWER			2
#define	 VAL_ROMANBIG 			3
#define	 VAL_ROMANSMALL 		4
#define	 VAL_DISC 				5
#define	 VAL_CIRCLE				6
#define	 VAL_SQUARE				7
#define	 VAL_CHECKBOX 			8
#define	 VAL_HIDDEN 			9
#define	 VAL_IMAGE 				10
#define	 VAL_RADIO 				11
#define	 VAL_RESET 				12
#define	 VAL_SUBMIT 			13
#define	 VAL_PASSWORD 			14
#define	 VAL_TEXT	 			15
#define  VAL_LEFT				16
#define  VAL_RIGHT				17
#define  VAL_CENTER				18
#define  VAL_CLEAR_ALL			19
#define  VAL_CLEAR_LINK			20
#define  VAL_INDENT				21
#define  VAL_MIDDLE				22
#define  VAL_ABSMIDDLE			23
#define  VAL_POST				24
#define  VAL_GET				25
#define  VAL_XCHCMD				26
#define  VAL_FORMS				27
#define  VAL_PLUGINS			28
#define	 VAL_TOP				29
#define	 VAL_BOTTOM				30
#define	 VAL_ABOVE				31
#define	 VAL_BELOW				32
#define	 VAL_LEFTOF				33
#define	 VAL_RIGHTOF			34
#define  VAL_IMAGES				35

// these are not attribute type values. These are used to determine the
// type of control based on other form attributes

#define  TYPE_MULTILINETEXT		100
#define  TYPE_POPUPLIST			101
#define  TYPE_LIST				102
#define  TYPE_LISTMULTI			103
#define  TYPE_LIST_ELEMENT		104
#define  TYPE_LIST_ELEMENT_SEL	105








/*----------------------------------------------------------------------------
	Types
----------------------------------------------------------------------------*/
// Entity and argument map table definition
typedef struct tagSymbolMap
{
	char*		pstrSymbol;
	int			iMap;

} SymbolMap, FAR* pSymbolMap;


// Special character map table
typedef struct tagCharType
{
	int		iCharType;
	int		chCharMap;

} CharType, FAR* pCharType;	


/*----------------------------------------------------------------------------
	Extern Variables :
----------------------------------------------------------------------------*/

// $Log$

#endif //  (!defined( _CHHTMSYM_H ))
