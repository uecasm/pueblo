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

	This file consists ndbm implementation header file.

----------------------------------------------------------------------------*/

// $Header$

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)ndbm.h	5.1 (Berkeley) 5/30/85
 */

// $Header$

#if !defined( _CHNDBM_H )
#define _CHNDBM_H


/*----------------------------------------------------------------------------
	Constants:
----------------------------------------------------------------------------*/

#define PBLKSIZ 1024
#define DBLKSIZ 4096


/*
 * flags to dbm_store()
 */
#define DBM_INSERT	0
#define DBM_REPLACE	1


/*----------------------------------------------------------------------------
	Types:
----------------------------------------------------------------------------*/

typedef struct tagDBM
{
	int		dbm_dirf;		/* open directory file */
	int		dbm_pagf;		/* open page file */
	//char 	dbm_dirf[MAX_PATH];
	//char 	dbm_pagf[MAX_PATH];
	chint32	dbm_flags;		/* flags, see below */
	long	dbm_maxbno;		/* last ``bit'' in dir file */
	long	dbm_bitno;		/* current bit number */
	long	dbm_hmask;		/* hash mask */
	long	dbm_blkptr;		/* current block for dbm_nextkey */
	chint32	dbm_keyptr;		/* current key for dbm_nextkey */
	long	dbm_blkno;		/* current page to read/write */
	long	dbm_pagbno;		/* current page in pagbuf */
	char	dbm_pagbuf[PBLKSIZ];	/* page file block buffer */
	long	dbm_dirbno;		/* current block in dirbuf */
	char	dbm_dirbuf[DBLKSIZ];	/* directory file block buffer */

} DBM, FAR *pDBM;

typedef struct tagDatum
{
	ptr			dptr;
	chint32		dsize;
} datum, FAR *pDatum;


/*----------------------------------------------------------------------------
	Functions:
----------------------------------------------------------------------------*/

CH_EXTERN_FUNC(DBM *)
dbm_open(char *, int, int );

CH_EXTERN_FUNC( void )
dbm_close(DBM *);

CH_EXTERN_FUNC( datum )
dbm_fetch(DBM *, datum);

CH_EXTERN_FUNC( datum )
dbm_firstkey(DBM *);

CH_EXTERN_FUNC( datum )
dbm_nextkey(DBM *);

CH_EXTERN_FUNC( long )
dbm_forder(DBM *, datum);

CH_EXTERN_FUNC( chint32 )
dbm_delete(DBM *, datum);

CH_EXTERN_FUNC( chint32 )
dbm_store(DBM *, datum, datum, chint32);


/*----------------------------------------------------------------------------
	Macros:
----------------------------------------------------------------------------*/

#define _DBM_RDONLY	0x1	/* data base open read-only */
#define _DBM_IOERR	0x2	/* data base I/O error */


#define dbm_error(db)	((db)->dbm_flags & _DBM_IOERR)
	/* use this one at your own risk! */
#define dbm_clearerr(db)	((db)->dbm_flags &= ~_DBM_IOERR)

// $Log$

#endif	// !defined( _CHNDBM_H )
