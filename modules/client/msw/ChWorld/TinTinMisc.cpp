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

	TinTin class miscellaneous methods.  Originally modified from TinTin++,
	(T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t, originally coded by
	Peter Unold 1992.

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"
#include <ChCore.h>
#include <ChDlg.h>

#include <mmsystem.h>

#include "headers.h"
#include "World.h"
#include "ChTextInput.h"
#include "ChTextOutput.h"

#include "TinTin.h"
#include "TinTinInfo.h"
#include "MemDebug.h"


/*----------------------------------------------------------------------------
	TinTin class protected methods
----------------------------------------------------------------------------*/

void TinTin::DoAlert()
{
	GetMainInfo()->DoAlert();
}


void TinTin::DoBell()
{
	MessageBeep( MB_OK );
}


/*----------------------------------------------------------------------------
	TinTin::DoChar
				Changes the TinTin escape character (start of commands.)
----------------------------------------------------------------------------*/

void TinTin::DoChar( const ChString& strArgs )
{
	ChString		strWorking;
	ChString		strMessage;

	GetArgInBraces( strArgs, strWorking, true );

	if ('\\' == strWorking[0])
	{
		LOADSTRING( IDS_TINTIN_CHAR_BACKSLASH_ERR, strMessage );
		ErrMessage( strMessage );
	}
	else if (ispunct( strWorking[0] ))
	{
		if (strWorking[0] != GetTinTinChar())
		{
			ChString		strFormat;

			SetTinTinChar( strWorking[0] );

			LOADSTRING( IDS_TINTIN_CHAR_OK, strFormat );
			strMessage.Format( strFormat, GetTinTinChar() );

			ErrMessage( strMessage );
		}
	}
	else
	{
		LOADSTRING( IDS_TINTIN_CHAR_ERR, strMessage );
		ErrMessage( strMessage );
	}
}


void TinTin::DoCR()
{
	Send( "\n" );
}


void TinTin::DoEnd( const ChString& strCommand )
{
	ChString		strMessage;

	if (0 != strCommand.Compare( "end" ))
	{
		LOADSTRING( IDS_TINTIN_END, strMessage );
		ErrMessage( strMessage );
	}
	else
	{
		ChPersistentFrame*	pFrame = GetMainInfo()->GetCore()->GetFrameWnd();
		ChString				strTitle;

		LOADSTRING( IDS_TINTIN_END_VERIFY, strMessage );
		LOADSTRING( IDS_TINTIN_END_VERIFY_TITLE, strTitle );
		if (IDYES == pFrame->MessageBox( strMessage, strTitle,
											MB_YESNO | MB_ICONEXCLAMATION ))
		{
			pFrame->PostMessage( WM_CLOSE );
		}
	}
}


void TinTin::DoHelp( const ChString& strArgs )
{
	ChString		strTopic;
	DWORD		dwKey = 0;

	GetArgStopAtSpaces( strArgs, strTopic );

	if (strTopic.IsEmpty())
	{
		strTopic = TINTIN_HELP_PREFIX ":overview";
		dwKey = (DWORD)(const char*)strTopic;
	}
	else
	{
		TTCommand	cmd;

		cmd = ParseTinTinCmd( strTopic );

		if (ttUnknown == cmd)
		{
			ChString		strFormat( "# Unknown command!  ('%s')" );
			ChString		strStatus;

			strStatus.Format( strFormat, (const char*)strTopic );
			ErrMessage( strStatus );
		}
		else
		{
			strTopic = m_pstrCommands[(int)cmd];

			strTopic = TINTIN_HELP_PREFIX ":#" + strTopic;
			dwKey = (DWORD)(const char*)strTopic;
		}
	}

	if (dwKey)
	{
		AfxGetApp()->WinHelp( dwKey, HELP_KEY );
	}
}


void TinTin::DoInfo()
{
	TinTinInfo		info;

	info.SetCounts( GetListActions()->GetCount(),
					GetListAliases()->GetCount(),
					GetListSubs()->GetCount(),
					GetListAntiSubs()->GetCount(),
					GetListVars()->GetCount(),
					GetListHighlights()->GetCount() );
	info.SetFlags( IsIgnore(), IsSpeedwalk(),
					IsUsingSubs(), IsUsingPresubs() );

	info.DoModal();
}


/*----------------------------------------------------------------------------
	TinTin::DoKillAll
				Empties the lists.
----------------------------------------------------------------------------*/

void TinTin::DoKillAll( bool boolQuiet )
{
	if (GetSettings())
	{
		GetSettings()->DoKillAll();
	}
	
	if (GetListPath())
	{
		GetListPath()->Empty();
	}

	if (!boolQuiet)
	{
		ChString		strMessage;

		LOADSTRING( IDS_TINTIN_KILLALL, strMessage );
		Message( strMessage );
	}
}


void TinTin::DoLog( const ChString& strArgs )
{
	ChString			strFilepath;
	ChTextOutput*	pTextOutput = GetMainInfo()->GetTextOutput();

											// Validate the path

	GetArgInBraces( strArgs, strFilepath, false );

	if (strFilepath.IsEmpty())
	{										// Turn off logging
		ChString		strMessage;

		if (pTextOutput->IsLogging())
		{
			pTextOutput->ToggleLogging();
		}

		LOADSTRING( IDS_TINTIN_LOG_OFF, strMessage );
		Message( strMessage );
	}
	else
	{
		ChString		strFormat;
		ChString		strMessage;
		bool		boolWasLogging = pTextOutput->IsLogging();

		if (pTextOutput->SetLogging( strFilepath ))
		{
			LOADSTRING( IDS_TINTIN_LOG_ON, strFormat );
			strMessage.Format( strFormat, (const char*)strFilepath );
			Message( strMessage );
		}
		else
		{
			LOADSTRING( IDS_TINTIN_LOG_ERROR, strFormat );
			strMessage.Format( strFormat, (const char*)strFilepath );
			ErrMessage( strMessage );

			if (boolWasLogging)
			{
				LOADSTRING( IDS_TINTIN_LOG_OFF, strMessage );
				ErrMessage( strMessage );
			}
		}
	}
}


void TinTin::DoLoop( const ChString& strArgs )
{
	const char*	pstrArgs = strArgs;
	ChString		strLeft;
	ChString		strRight;
	int			iBound1;
	int			iBound2;

	pstrArgs = GetArgInBraces( pstrArgs, strLeft, false );
	pstrArgs = GetArgInBraces( pstrArgs, strRight, true );

	if (sscanf( (const char*)strLeft, "%d,%d", &iBound1, &iBound2 ) != 2)
	{
		ChString		strMessage;

		LOADSTRING( IDS_TINTIN_LOOP_PARAM_ERR, strMessage );
		ErrMessage( strMessage );
	}
	else
	{
		bool	boolDone = false;
		int		iCounter = iBound1;

		while(!boolDone)
		{
			ChString		strResult;

			m_strVars[0].Format( "%d", iCounter );

			SubstituteVars( strRight, strResult );
			ParseInput( strResult );

			if (iBound1 < iBound2)
			{
				iCounter++;
				if (iCounter > iBound2)
				{
					boolDone = true;
				}
			}
			else
			{
				iCounter--;
				if (iCounter < iBound2)
				{
					boolDone = true;
				}
			}
		}
	}
}


void TinTin::DoMbox( const ChString& strArgs )
{
	ChString		strResult;
	ChString		strMessageTitle;

	GetArgInBraces( strArgs, strResult, true );

	PrepareActionAlias( strResult, strResult );

	LOADSTRING( IDS_TINTIN_MBOX_TITLE, strMessageTitle );
	GetMainInfo()->GetCore()->GetFrameWnd()->
				MessageBox( strResult, strMessageTitle );
}


void TinTin::DoMessage( const ChString& strArgs )
{
	static const char*	pstrNames[] = { "actions",
										"aliases",
										"antisubstitutes",
										"highlights",
										"pathdirs",
										"substitutes",
										"variables" };
	static const char*	pstrOff = "off";
	static const char*	pstrOn = "on";

	int					iOption;
	ChString				strOption;
	ChString				strFormat;
	ChString				strMessage;

	GetArgInBraces( strArgs, strOption, true );

	iOption = 0;
	while (!IsAbrev( strOption, pstrNames[iOption] ) &&
			(iOption < msgIndexLast))
	{
		iOption++;
	}

	if (iOption == msgIndexLast)
	{
		LOADSTRING( IDS_TINTIN_MESSAGE_ERR, strFormat );
		strMessage.Format( strFormat, (const char*)strOption );
		ErrMessage( strMessage );
	}
	else
	{
		SetDisplayingMsg( iOption, !IsDisplayingMsg( iOption ) );

		LOADSTRING( IDS_TINTIN_MESSAGE_OK, strFormat );
		strMessage.Format( strFormat, pstrNames[iOption],
							IsDisplayingMsg( iOption ) ? pstrOn :
																pstrOff );
		ErrMessage( strMessage );
	}
}


void TinTin::DoPlaySound( const ChString& strArgs, bool async )
{
	ChString			strFilepath;
											// Validate the path

	GetArgInBraces( strArgs, strFilepath, false );

	if (strFilepath.IsEmpty())
	{										// Turn off logging
		ChString		strMessage;

		LOADSTRING( IDS_TINTIN_PLAY_EMPTY, strMessage );
		ErrMessage( strMessage );
	}
	else
	{
		DWORD flags = SND_FILENAME;
		if(async) flags |= SND_ASYNC;
		if (!PlaySound( strFilepath, 0, flags ))
		{
			ChString		strFormat;
			ChString		strMessage;

			LOADSTRING( IDS_TINTIN_PLAY_ERROR, strFormat );
			strMessage.Format( strFormat, (const char*)strFilepath );
			ErrMessage( strMessage );
		}
	}
}


void TinTin::DoRandom( const ChString& strArgs )
{
	const char*		pstrArgs = strArgs;
	ChString			strVar;
	ChString			strMax;
	ChString			strTemp;

	pstrArgs = GetArgInBraces( pstrArgs, strVar, false );
	pstrArgs = GetArgInBraces( pstrArgs, strMax, true );

	SubstituteVars( strMax, strTemp );
	SubstituteMyVars( strTemp, strMax );

	if (!strVar.IsEmpty() && !strMax.IsEmpty())
	{
		const char*		pstrMax = strMax;
		bool			boolSuccess = false;

		if (isdigit( *pstrMax ))
		{
			chint32		lMax = atol( pstrMax );

			if ((lMax > 0) && ((chuint32)lMax <= RAND_MAX))
			{
				chint32			lValue;
				TinTinListNode*	pNode;
											/* Generate a random number between
												0 and lMax */
				lValue = rand();
				lValue = (lValue * (lMax + 1)) / RAND_MAX;

				strTemp.Format( "%ld", lValue );

				if (pNode = GetListVars()->Search( strVar ))
				{
					GetListVars()->DeleteNode( pNode );
				}

				GetListVars()->InsertNode( strVar, strTemp, "0" );
				boolSuccess = true;
			}
		}

		if (!boolSuccess)
		{
			ChString	strMessage;
			ChString	strFormat;

			LOADSTRING( IDS_TINTIN_RANDOM_SIZE_ERR, strFormat );
			strMessage.Format( strFormat, (chint32)RAND_MAX );
			Message( strMessage );
		}
	}
	else
	{
		ChString	strMessage;

		LOADSTRING( IDS_TINTIN_RANDOM_USAGE, strMessage );
		Message( strMessage );
	}
}


void TinTin::DoShowMe( const ChString& strArgs )
{
	ChString	strResult;

	GetArgInBraces( strArgs, strResult, true );

	PrepareActionAlias( strResult, strResult );
	Message( strResult );
}

void TinTin::DoShowMeHtml( const ChString& strArgs )
{
	ChString	strResult;

	GetArgInBraces( strArgs, strResult, true );

	PrepareActionAlias( strResult, strResult );
	if (IsMessages())
	{
		Display( strResult, false, true );
	}
}

/*----------------------------------------------------------------------------
	TinTin::DoSplit
				Sets the size of the output or input windows, dependant on
				the argument.  The argument is an integer, whose
				interpretation is as follows:

				Argument:	Result:
				---------	--------------------------------------------------
					<0		Sets the number of lines for the input window.
					0		Does nothing.
					>0		Sets the number of lines for the input window to
							(24 - arg).
----------------------------------------------------------------------------*/

void TinTin::DoSplit( const ChString& strArgs )
{
	if (!strArgs.IsEmpty())
	{
		int		iCount = atoi( strArgs );
		bool	boolValid = false;

		if (iCount < 0)
		{
			iCount = -iCount;
			boolValid = true;
		}
		else if ((iCount > 0) && (iCount < 24))
		{									/* The number of lines they want
												for the output window.  Assume
												a 24-line screen and set the
												number of input lines
												appropriately */
			iCount = 24 - iCount;
			boolValid = true;
		}

		if (boolValid)
		{
			GetMainInfo()->GetTextInput()->SetInputLines( iCount );
		}
	}
}


void TinTin::DoVersion()
{
	ChString		strFormat;
	ChString		strMessage;

	LOADSTRING( IDS_TINTIN_VERSION, strFormat );
	strMessage.Format( strFormat, TINTIN_VERSION );
	Message( strMessage );
}


void TinTin::DoWizList()
{
	ChString		strMessage;

	LOADSTRING( IDS_TINTIN_WIZLIST, strMessage );
	Message( strMessage );
}


void TinTin::DoZap()
{
	ChString		strMessage;

	LOADSTRING( IDS_TINTIN_ZAP, strMessage );
	Message( strMessage );

	GetMainInfo()->ShutdownWorld();
}


/*----------------------------------------------------------------------------
	TinTin::ToggleEcho
				Toggles the 'echo' state.  If 'echo' is true, then
				actions will be echoed to the screen.
----------------------------------------------------------------------------*/

void TinTin::ToggleEcho()
{
	ChString		strMessage;

	SetEcho( !IsEcho() );

	if (IsEcho())
	{
		LOADSTRING( IDS_TINTIN_ECHO_ON, strMessage );
	}
	else 
	{
		LOADSTRING( IDS_TINTIN_ECHO_OFF, strMessage );
	}

	Message( strMessage );
}


/*----------------------------------------------------------------------------
	TinTin::ToggleIgnore
				Toggles the 'ignore' state.  If 'ignore' is true, then
				actions will not occur.
----------------------------------------------------------------------------*/

void TinTin::ToggleIgnore()
{
	ChString		strMessage;

	SetIgnore( !IsIgnore() );

	if (IsIgnore())
	{
		LOADSTRING( IDS_TINTIN_IGNORE_ON, strMessage );
	}
	else 
	{
		LOADSTRING( IDS_TINTIN_IGNORE_OFF, strMessage );
	}

	Message( strMessage );
}


/*----------------------------------------------------------------------------
	TinTin::ToggleSpeedwalk
				Toggles the 'speedwalk' state.
----------------------------------------------------------------------------*/

void TinTin::ToggleSpeedwalk()
{
	ChString		strMessage;

	SetSpeedwalk( !IsSpeedwalk() );

	if (IsSpeedwalk())
	{
		LOADSTRING( IDS_TINTIN_SPEEDWALK_ON, strMessage );
	}
	else 
	{
		LOADSTRING( IDS_TINTIN_SPEEDWALK_OFF, strMessage );
	}

	Message( strMessage );
}


/*----------------------------------------------------------------------------
	TinTin::ToggleSub
				Toggles whether substitutions occur.
----------------------------------------------------------------------------*/

void TinTin::ToggleSub()
{
	ChString		strMessage;

	SetUsingSubs( !IsUsingSubs() );
	if (IsUsingSubs())
	{
		LOADSTRING( IDS_TINTIN_SUBS_ON, strMessage );
	}
	else 
	{
		LOADSTRING( IDS_TINTIN_SUBS_OFF, strMessage );
	}

	Message( strMessage );
}


/*----------------------------------------------------------------------------
	TinTin::ToggleVerbatim
				Toggles the verbatim state.  If 'verbatim' is true, then
				all text is sent 'as is' to the world.

				strArg may be 'on' or 'off' to set the mode explicitly.
----------------------------------------------------------------------------*/

void TinTin::ToggleVerbatim( const ChString& strArg )
{
	ChString		strMode( strArg );
	bool		boolError = false;

	strMode.MakeLower();

	if (strMode.IsEmpty())
	{
		SetVerbatim( !IsVerbatim() );
	}
	else if ("on" == strMode)
	{
		SetVerbatim( true );
	}
	else if ("off" == strMode)
	{
		SetVerbatim( false );
	}
	else
	{
		ChString		strMessage;

		LOADSTRING( IDS_TINTIN_VERBATIM_PARAM_ERR, strMessage );
		ErrMessage( strMessage );
		boolError = true;
	}

	if (!boolError)
	{
		ChString		strMessage;

		if (IsVerbatim())
		{
			LOADSTRING( IDS_TINTIN_VERBATIM_ON, strMessage );
		}
		else 
		{
			LOADSTRING( IDS_TINTIN_VERBATIM_OFF, strMessage );
		}

		Message( strMessage );
	}
}



#if 0

/* Autoconf patching by David Hedbor, neotron@lysator.liu.se */
/*********************************************************************/
/* file: misc.c - misc commands                                      */
/*                             TINTIN III                            */
/*          (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t             */
/*                     coded by peter unold 1992                     */
/*********************************************************************/
#ifdef HAVE_STRING_H
#include <string.h>
#else
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#endif
#include <ctype.h>
#include "tintin.h"

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

/* externs */
extern struct session *newactive_session();
extern struct listnode *common_aliases, *common_actions, *common_subs, *common_myvars;
extern struct listnode *common_highs, *common_antisubs, *common_pathdirs;
extern char *get_arg_in_braces();
extern void term_echo();
extern int redraw, is_split;
extern struct session *sessionlist;
extern struct completenode *complete_head;
extern char tintin_char;
extern int echo;
extern int speedwalk;
extern int presub;
extern int togglesubs;
extern char vars[10][BUFFER_SIZE]; /* the %0, %1, %2,....%9 variables */
extern int mesvar[7];
extern int verbatim;

/**********************/
/* the #presub command*/
/**********************/
void presub_command(ses)
     struct session *ses;
{
  presub=!presub;
  if(presub)
    tintin_puts("#ACTIONS ARE NOW PROCESSED ON SUBSTITUTED BUFFER.",ses);
  else
    tintin_puts("#ACTIONS ARE NO LONGER DONE ON SUBSTITUTED BUFFER.",ses);
}

/**************************/
/* the #togglesubs command*/
/**************************/
void togglesubs_command(ses)
     struct session *ses;
{
  togglesubs=!togglesubs;
  if(togglesubs)
    tintin_puts("#SUBSTITUTES ARE NOW IGNORED.",ses);
  else 
    tintin_puts("#SUBSTITUTES ARE NO LONGER IGNORED.", ses);
}

/**********************/
/* the #snoop command */
/**********************/
void snoop_command(arg, ses)
     char *arg;
     struct session *ses;
{
  char buf[100];
  struct session *sesptr=ses;

  if(ses) {
    get_arg_in_braces(arg, arg,1);
    if(*arg) {
      for(sesptr=sessionlist; sesptr && strcmp(sesptr->name, arg); sesptr=sesptr->next);
      if(!sesptr) {
        tintin_puts("#NO SESSION WITH THAT NAME!", ses);
        return;
      }
    }
    if(sesptr->snoopstatus) {
      sesptr->snoopstatus=FALSE;
      sprintf(buf, "#UNSNOOPING SESSION '%s'", sesptr->name);
      tintin_puts(buf, ses);
    }
    else {
      sesptr->snoopstatus=TRUE;
      sprintf(buf, "#SNOOPING SESSION '%s'", sesptr->name);
      tintin_puts(buf, ses);
    }
  }
  else
    tintin_puts("#NO SESSION ACTIVE => NO SNOOPING", ses);
}


/***********************/
/* the #system command */
/***********************/
void system_command(arg, ses)
     char *arg;
     struct session *ses;
{
  get_arg_in_braces(arg, arg, 1);
  if(*arg) {
    tintin_puts3("^#OK EXECUTING SHELL COMMAND.", ses);
    term_echo();
#if defined(DEBUG)
    system(arg);
#else
    alarm(0);
    system(arg);
#endif
#if defined(DEBUG)
    alarm(0);
#else
    alarm(1);
#endif
    term_noecho();
    tintin_puts3("!#OK COMMAND EXECUTED.", ses);
  }
  else
    tintin_puts2("#EXECUTE WHAT COMMAND?", ses);
  prompt(NULL);

}



/*********************************************************************/
/*   tablist will display the all items in the tab completion file   */
/*********************************************************************/
void tablist(tcomplete)
     struct completenode *tcomplete;
{
   int count, done;
   char tbuf[BUFFER_SIZE];
   struct completenode *tmp;

done=0;
if (tcomplete==NULL)
  {
   tintin_puts2("Sorry.. But you have no words in your tab completion file",NULL);
   return;
  }
count=1;
*tbuf='\0';

/* 
   I'll search through the entire list, printing thre names to a line then
   outputing the line.  Creates a nice 3 column effect.  To increase the # 
   if columns, just increase the mod #.  Also.. decrease the # in the %s's
*/

for(tmp=tcomplete->next;tmp!=NULL;tmp=tmp->next) 
  {
  if ((count % 3))
     {
     if (count == 1)
        sprintf(tbuf,"%25s", tmp->strng); 
     else
        sprintf(tbuf,"%s%25s",tbuf,tmp->strng);
     done=0;
     ++count;
     }
  else
     {
     sprintf(tbuf,"%s%25s",tbuf,tmp->strng);
     tintin_puts2(tbuf, NULL);
     done=1;
     *tbuf='\0';
     ++count;
     }
  }
  if (!done)
     tintin_puts2(tbuf, NULL);
  prompt(NULL);
}

void tab_add(arg)
   char *arg;
{
   struct completenode *tmp, *tmpold, *tcomplete;
   struct completenode *newt;
   char *newcomp, buff[BUFFER_SIZE];

   tcomplete=complete_head;
  
   if ((arg == NULL) || (strlen(arg)<=0)) {
     tintin_puts("Sorry, you must have some word to add.", NULL);
     prompt(NULL);
     return;
   }
   get_arg_in_braces(arg, buff, 1);

   if ((newcomp=(char *)(malloc(strlen(buff)+1)))==NULL) {
       fprintf(stderr, "Could not allocate enough memory for that Completion word.\n");
       exit(1);
   }
   strcpy(newcomp, buff);
   tmp=tcomplete;
   while (tmp->next != NULL) {
      tmpold=tmp;
      tmp=tmp->next;
   } 

   if ((newt=(struct completenode *)(malloc(sizeof(struct completenode))))==NULL) {
       fprintf(stderr, "Could not allocate enough memory for that Completion word.\n");
       exit(1);
   }
       
   newt->strng=newcomp;
   newt->next=NULL;
   tmp->next=newt;
   tmp=newt;
   sprintf(buff,"#New word %s added to tab completion list.", arg);
   tintin_puts(buff, NULL);
   prompt(NULL);
}
   
void tab_delete(arg)
   char *arg;
{
   struct completenode *tmp, *tmpold, *tmpnext, *tcomplete;
   struct completenode *newt;
   char *oldcomp, s_buff[BUFFER_SIZE], c_buff[BUFFER_SIZE];

   tcomplete=complete_head;

   if ((arg == NULL) || (strlen(arg)<=0)) {
     tintin_puts("#Sorry, you must have some word to delete.", NULL);
     prompt(NULL);
     return;
   }
   get_arg_in_braces(arg, s_buff, 1);
   tmp=tcomplete->next;
   tmpold=tcomplete;
   if (tmpold->strng == NULL) {   /* (no list if the second node is null) */
     tintin_puts("#There are no words for you to delete!", NULL);
     prompt(NULL);
     return;
   } 
   strcpy(c_buff, tmp->strng); 
   while ((tmp->next != NULL) && (strcmp(c_buff, s_buff) != 0)) {
      tmpold=tmp;
      tmp=tmp->next;
      strcpy(c_buff, tmp->strng); 
   }
   if (tmp->next != NULL) {
      tmpnext=tmp->next;
      tmpold->next=tmpnext;
      free(tmp);
      tintin_puts("#Tab word deleted.", NULL);
      prompt(NULL);
    } else {
      if (strcmp(c_buff, s_buff) == 0) { /* for the last node to delete */
        tmpold->next=NULL;
        free(tmp);
        tintin_puts("#Tab word deleted.", NULL);
        prompt(NULL);
        return;
      }
      tintin_puts("Word not found in list.", NULL);
      prompt(NULL);
    }
}    
 
#endif	// 0

// $Log$
// Revision 1.2  2003/07/04 11:26:42  uecasm
// Update to 2.60 (see help file for details)
//
// Revision 1.1.1.1  2003/02/03 18:53:39  uecasm
// Import of source tree as at version 2.53 release.
//
