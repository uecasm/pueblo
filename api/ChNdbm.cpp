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

	ndbm database implementation

----------------------------------------------------------------------------*/

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include "headers.h"

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)ndbm.c	5.4 (Berkeley) 9/4/87";
#endif //LIBC_SCCS and not lint

#include "headers.h"


#include <fcntl.h>
#ifdef CH_UNIX
#include <unistd.h>
#define _O_RDONLY O_RDONLY
#define _O_WRONLY O_WRONLY
#define _O_RDWR O_RDWR
#define _open open
#define _fstat fstat
#define _close close
#define _lseek lseek
#define _read read
#define _write write
#define _commit(foo) 
#endif
#include <sys/types.h>
#include <sys/stat.h>
//#include <sys/file.h>
#include <stdlib.h>
#include <memory.h>
#ifdef CH_MSW
#include <io.h>
#endif
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <ChTypes.h>

#include "ChNdbm.h"

#include <MemDebug.h>


/*----------------------------------------------------------------------------
	Constants:
----------------------------------------------------------------------------*/

#define BYTESIZ 8
#undef setbit


#define dbm_rdonly(db)	((db)->dbm_flags & _DBM_RDONLY)

/* for flock(2) and fstat(2) */
#define dbm_dirfno(db)	((db)->dbm_dirf)
#define dbm_pagfno(db)	((db)->dbm_pagf)





/*----------------------------------------------------------------------------
	Types:
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
	Variables:
----------------------------------------------------------------------------*/

static  datum makdatum();
static  long finddatum();
static	chint32 additem();
static	chint32 delitem();
static  long dcalchash();

#if defined( CH_ARCH_32 )
CH_INTERN_VAR HANDLE			hMutex = 0;
#define CH_DB_MUTEX_NAME		"ChacoDBSyncMutex"
#endif

/*----------------------------------------------------------------------------
	Functions:
----------------------------------------------------------------------------*/

CH_INTERN_FUNC( chint32 )
getbit(register DBM *db);

CH_INTERN_FUNC( void )
setbit(register DBM *db);

CH_INTERN_FUNC( void )
dbm_access(register DBM *db, long hash);

CH_INTERN_FUNC( datum )
makdatum(char buf[PBLKSIZ], long n);

CH_INTERN_FUNC( long )
finddatum(char buf[PBLKSIZ], datum item);

CH_INTERN_FUNC( long )
dcalchash(datum item);

CH_INTERN_FUNC( chint32 )
delitem(char buf[PBLKSIZ], long n);


CH_INTERN_FUNC( chint32 )
additem(char buf[PBLKSIZ], datum item, datum item1);

#ifdef DEBUG
CH_INTERN_FUNC( chint32 )
chkblk(char buf[PBLKSIZ]);
#endif

CH_INTERN_FUNC( int )
ReadData( int iFile, long pos, void* pBuf, long lBufSize );

CH_INTERN_FUNC( int )
WriteData( int iFile, long pos, void* pBuf, long lBufSize );




/*----------------------------------------------------------------------------
	Macros:
----------------------------------------------------------------------------*/


CH_GLOBAL_FUNC(DBM *)
dbm_open( pstr file, int flags, int mode)
{

#if defined( CH_MSW ) && defined( CH_ARCH_32 )
	if ( hMutex == 0)
	{
		hMutex = CreateMutex(  NULL, false, CH_DB_MUTEX_NAME );
		if ( GetLastError() == ERROR_ALREADY_EXISTS )
		{
			hMutex = OpenMutex( MUTEX_ALL_ACCESS | SYNCHRONIZE, false, CH_DB_MUTEX_NAME );
		}
	}
#endif

#ifdef CH_MSW
	struct 		_stat statb;
#else
	struct	stat statb;
#endif
	register 	DBM *db;

    db = (DBM *)new DBM;

	if ( db == 0)
	{
		errno = ENOMEM;
		return ((DBM *)0);
	}

	db->dbm_flags = (flags & 03) == _O_RDONLY ? _DBM_RDONLY : 0;
	if ((flags & 03) == _O_WRONLY)
	{
		flags = (flags & ~03) | _O_RDWR;
	}

    #if defined( CH_MSW )
    lstrcpy(db->dbm_pagbuf, file);
    lstrcat(db->dbm_pagbuf, ".pag");
    #else
    strcpy(db->dbm_pagbuf, file);
    strcat(db->dbm_pagbuf, ".pag");
    #endif

		// UE: db->dbm_pagf = _open(db->dbm_pagbuf, flags, mode);
		db->dbm_pagf = open(db->dbm_pagbuf, flags, mode);

	if (db->dbm_pagf < 0)
		goto bad;

    #if defined( CH_MSW )
    lstrcpy(db->dbm_pagbuf, file);
    lstrcat(db->dbm_pagbuf, ".dir");
    #else
    strcpy(db->dbm_pagbuf, file);
    strcat(db->dbm_pagbuf, ".dir");
    #endif

    // UE: db->dbm_dirf = _open(db->dbm_pagbuf, flags, mode);
    db->dbm_dirf = open(db->dbm_pagbuf, flags, mode);

    if (db->dbm_dirf < 0)
            goto bad1;

    _fstat(db->dbm_dirf, &statb);
    db->dbm_maxbno = statb.st_size*BYTESIZ-1;
    db->dbm_pagbno = db->dbm_dirbno = -1;


	return (db);
bad1:
	(void) _close(db->dbm_pagf);
bad:
	delete((char *)db);
	return ((DBM *)0);
}

CH_GLOBAL_FUNC( void )
dbm_close(DBM *db)
{
	#if defined( CH_MSW ) && defined( CH_ARCH_32 )
	{
		if ( hMutex )
		{
			CloseHandle( hMutex );
			hMutex = 0;
		}
	}
	#endif	// defined( CH_MSW ) && defined( CH_ARCH_32 )

	(void) _close(db->dbm_dirf);
	(void) _close(db->dbm_pagf);
	delete((char *)db);
}

CH_INTERN_FUNC( chint32 )
getbit(register DBM *db)
{
	long bn;
	register long b, i, n;


	if (db->dbm_bitno > db->dbm_maxbno)
		return (0);

	n = db->dbm_bitno % BYTESIZ;
	bn = db->dbm_bitno / BYTESIZ;
	i = bn % DBLKSIZ;
	b = bn / DBLKSIZ;

	if (b != db->dbm_dirbno)
	{
		db->dbm_dirbno = b;

		if ( ReadData( db->dbm_dirf, (long)b*DBLKSIZ, db->dbm_dirbuf, DBLKSIZ  ) != DBLKSIZ )
		{
			ChMemClear(db->dbm_dirbuf, DBLKSIZ);

		}
		//(void) _lseek(db->dbm_dirf, (long)b*DBLKSIZ, SEEK_SET );

		//if (read(db->dbm_dirf, db->dbm_dirbuf, DBLKSIZ) != DBLKSIZ)
		//	ChMemClear(db->dbm_dirbuf, DBLKSIZ);
	}
	return (db->dbm_dirbuf[i] & (1<<n));
}

CH_INTERN_FUNC( void )
setbit(register DBM *db)
{
	long bn;
	register long i, n, b;

	if (db->dbm_bitno > db->dbm_maxbno)
		db->dbm_maxbno = db->dbm_bitno;

	n = db->dbm_bitno % BYTESIZ;
	bn = db->dbm_bitno / BYTESIZ;
	i = bn % DBLKSIZ;
	b = bn / DBLKSIZ;
	if (b != db->dbm_dirbno)
	{
		db->dbm_dirbno = b;
		if ( ReadData( db->dbm_dirf, (long)b*DBLKSIZ, db->dbm_dirbuf, DBLKSIZ  ) != DBLKSIZ )  
		{
			ChMemClear(db->dbm_dirbuf,  DBLKSIZ);
		}

		//(void) _lseek(db->dbm_dirf, (long)b*DBLKSIZ, SEEK_SET );
		//if (_read(db->dbm_dirf, db->dbm_dirbuf, DBLKSIZ) != DBLKSIZ)
		//	ChMemClear(db->dbm_dirbuf,  DBLKSIZ);
	}

	db->dbm_dirbuf[i] |= 1<<n;
	db->dbm_dirbno = b;

	if ( WriteData( db->dbm_dirf, (long)b*DBLKSIZ, db->dbm_dirbuf, DBLKSIZ  ) != DBLKSIZ )  
	{
		db->dbm_flags |= _DBM_IOERR;
	}


}

CH_GLOBAL_FUNC( long )
dbm_forder(register DBM *db, datum key )
{
	long hash;

	hash = dcalchash(key);

	for (db->dbm_hmask=0;; db->dbm_hmask=(db->dbm_hmask<<1)+1)
	{
		db->dbm_blkno = hash & db->dbm_hmask;
		db->dbm_bitno = db->dbm_blkno + db->dbm_hmask;
		if (getbit(db) == 0)
			break;
	}
	return (db->dbm_blkno);
}

CH_INTERN_FUNC( void )
dbm_access(register DBM *db, long hash)
{

	for (db->dbm_hmask=0;; db->dbm_hmask=(db->dbm_hmask<<1)+1)
	{
		db->dbm_blkno = hash & db->dbm_hmask;
		db->dbm_bitno = db->dbm_blkno + db->dbm_hmask;
		if (getbit(db) == 0)
			break;
	}
	if (db->dbm_blkno != db->dbm_pagbno)
	{
		db->dbm_pagbno = db->dbm_blkno;
		if ( ReadData( db->dbm_pagf, (long)db->dbm_blkno*PBLKSIZ, 
						db->dbm_pagbuf, PBLKSIZ  ) != PBLKSIZ )  
		{
			ChMemClear(db->dbm_pagbuf,  PBLKSIZ);
		}
		

#ifdef DEBUG
		else if (chkblk(db->dbm_pagbuf) < 0)
			db->dbm_flags |= _DBM_IOERR;
#endif
	}
}

CH_GLOBAL_FUNC( datum )
dbm_fetch(register DBM *db, datum key)
{
	register long i;
	datum item;

	if (dbm_error(db))
		goto err;

	dbm_access(db, dcalchash(key));

	if ((i = finddatum(db->dbm_pagbuf, key)) >= 0)
	{
		item = makdatum(db->dbm_pagbuf, i+1);
		if (item.dptr != NULL)
			return (item);
	}
err:
	item.dptr = NULL;
	item.dsize = 0;
	return (item);
}

CH_GLOBAL_FUNC( chint32 )
dbm_delete(register DBM *db, datum key)
{
	register long i;

	if (dbm_error(db))
		return (-1);

	if (dbm_rdonly(db)) {
		errno = EPERM;
		return (-1);
	}

	dbm_access(db, dcalchash(key));

	if ((i = finddatum(db->dbm_pagbuf, key)) < 0)
		return (-1);

	if (!delitem(db->dbm_pagbuf, i))
		goto err;

	db->dbm_pagbno = db->dbm_blkno;

	if ( WriteData( db->dbm_pagf, (long)db->dbm_blkno*PBLKSIZ, 
					db->dbm_pagbuf, PBLKSIZ  ) != PBLKSIZ )  
	{
		err:
			db->dbm_flags |= _DBM_IOERR;
			return (-1);
	}


	return (0);
}

CH_GLOBAL_FUNC( chint32 )
dbm_store(register DBM *db, datum key, datum dat, chint32 replace)
{
	register long i;
	datum item, item1;

	if (dbm_error(db))
		return (-1);

	if (dbm_rdonly(db))
	{
		errno = EPERM;
		return (-1);
	}
loop:
	dbm_access(db, dcalchash(key));
	if ((i = finddatum(db->dbm_pagbuf, key)) >= 0)
	{
		if (!replace)
			return (1);

		if (!delitem(db->dbm_pagbuf, i))
		{
			db->dbm_flags |= _DBM_IOERR;
			return (-1);
		}
	}
	if (!additem(db->dbm_pagbuf, key, dat))
		goto split;

	db->dbm_pagbno = db->dbm_blkno;


	if ( WriteData( db->dbm_pagf, (long)db->dbm_blkno*PBLKSIZ, 
					db->dbm_pagbuf, PBLKSIZ  ) != PBLKSIZ )  
	{
		db->dbm_flags |= _DBM_IOERR;
		return (-1);
	}
	
	return (0);

split:
	if (key.dsize+dat.dsize+3*sizeof(short) >= PBLKSIZ)
	{
		db->dbm_flags |= _DBM_IOERR;
		errno = ENOSPC;
		return (-1);
	}

	char* ovfbuf = new char[PBLKSIZ];

	ChMemClear(ovfbuf, PBLKSIZ);
	for (i=0;;)
	{
		item = makdatum(db->dbm_pagbuf, i);
		if (item.dptr == NULL)
			break;
		if (dcalchash(item) & (db->dbm_hmask+1))
		{
			item1 = makdatum(db->dbm_pagbuf, i+1);
			if (item1.dptr == NULL) {
				//fprintf(stderr, "ndbm: split not paired\n");
				db->dbm_flags |= _DBM_IOERR;
				break;
			}
			if (!additem(ovfbuf, item, item1) ||
			    !delitem(db->dbm_pagbuf, i)) {
				db->dbm_flags |= _DBM_IOERR;
				return (-1);
			}
			continue;
		}
		i += 2;
	}
	db->dbm_pagbno = db->dbm_blkno;

	if ( WriteData( db->dbm_pagf, (long)db->dbm_blkno*PBLKSIZ, 
					db->dbm_pagbuf, PBLKSIZ  ) != PBLKSIZ )  
	{
		db->dbm_flags |= _DBM_IOERR;
		delete []ovfbuf;
		return (-1);
	}
	

	if ( WriteData( db->dbm_pagf, (long)(db->dbm_blkno+db->dbm_hmask+1)*PBLKSIZ, 
					ovfbuf, PBLKSIZ  ) != PBLKSIZ )  
	{
		db->dbm_flags |= _DBM_IOERR;
		delete []ovfbuf;
		return (-1);
	}
	

	setbit(db);    
	delete []ovfbuf;
	goto loop;
}

CH_GLOBAL_FUNC( datum )
dbm_firstkey(DBM *db)
{

	db->dbm_blkptr = 0L;
	db->dbm_keyptr = 0;
	return (dbm_nextkey(db));
}

CH_GLOBAL_FUNC( datum )
dbm_nextkey(register DBM *db)
{
	datum			item;

	#if defined( CH_MSW )

	struct _stat	statb;

	#elif defined( CH_UNIX )

	struct stat		statb;

	#else

		#error( "Platform not defined!" );

	#endif

	if (dbm_error( db ) || _fstat( db->dbm_pagf, &statb ) < 0)
		goto err;

	statb.st_size /= PBLKSIZ;

	for (;;)
	{
		if (db->dbm_blkptr != db->dbm_pagbno)
		{
			db->dbm_pagbno = db->dbm_blkptr;

			if ( ReadData( db->dbm_pagf, (long)db->dbm_blkptr*PBLKSIZ, 
							db->dbm_pagbuf, PBLKSIZ  ) != PBLKSIZ )  
			{
				ChMemClear(db->dbm_pagbuf, PBLKSIZ);
			}

			//(void) _lseek(db->dbm_pagf, db->dbm_blkptr*PBLKSIZ, SEEK_SET );
			//if (_read(db->dbm_pagf, db->dbm_pagbuf, PBLKSIZ) != PBLKSIZ)
			//	ChMemClear(db->dbm_pagbuf, PBLKSIZ);
#ifdef DEBUG
			else if (chkblk(db->dbm_pagbuf) < 0)
				db->dbm_flags |= _DBM_IOERR;
#endif
		}
		if (((short *)db->dbm_pagbuf)[0] != 0)
		{
			item = makdatum(db->dbm_pagbuf, db->dbm_keyptr);
			if (item.dptr != NULL)
			{
				db->dbm_keyptr += 2;
				return (item);
			}
			db->dbm_keyptr = 0;
		}
		if (++db->dbm_blkptr >= statb.st_size)
			break;
	}
err:
	item.dptr = NULL;
	item.dsize = 0;
	return (item);
}

CH_INTERN_FUNC( datum )
makdatum(char buf[PBLKSIZ], long n)
{
	register short *sp;
	register t;
	datum item;

	sp = (short *)buf;
	if ((long)n >= sp[0])
	{
		item.dptr = NULL;
		item.dsize = 0;
		return (item);
	}
	t = PBLKSIZ;
	if (n > 0)
		t = sp[n];
	item.dptr = buf+sp[n+1];
	item.dsize = t - sp[n+1];
	return (item);
}

CH_INTERN_FUNC( long )
finddatum(char buf[PBLKSIZ], datum item)
{
	register short *sp;
	register chint32 i, n, j;

	sp = (short *)buf;
	n = PBLKSIZ;
	for (i=0, j=sp[0]; i<j; i+=2, n = sp[i])
	{
		n -= sp[i+1];
		if (n != item.dsize)
			continue;
		#if defined( CH_ARCH_16 )
		if (n == 0 || _fmemcmp(item.dptr, &buf[sp[i+1]], (int)n)  == 0)
			return (i);
		#else
		if (n == 0 ||  memcmp(item.dptr, &buf[sp[i+1]], n)  == 0)
			return (i);
		#endif
	}
	return (-1);
}

CH_INTERN_VAR  chint32 hitab[16]
= {    61, 57, 53, 49, 45, 41, 37, 33,
	29, 25, 21, 17, 13,  9,  5,  1,
};
CH_INTERN_VAR  long hltab[64]
 = {
	06100151277L,06106161736L,06452611562L,05001724107L,
	02614772546L,04120731531L,04665262210L,07347467531L,
	06735253126L,06042345173L,03072226605L,01464164730L,
	03247435524L,07652510057L,01546775256L,05714532133L,
	06173260402L,07517101630L,02431460343L,01743245566L,
	00261675137L,02433103631L,03421772437L,04447707466L,
	04435620103L,03757017115L,03641531772L,06767633246L,
	02673230344L,00260612216L,04133454451L,00615531516L,
	06137717526L,02574116560L,02304023373L,07061702261L,
	05153031405L,05322056705L,07401116734L,06552375715L,
	06165233473L,05311063631L,01212221723L,01052267235L,
	06000615237L,01075222665L,06330216006L,04402355630L,
	01451177262L,02000133436L,06025467062L,07121076461L,
	03123433522L,01010635225L,01716177066L,05161746527L,
	01736635071L,06243505026L,03637211610L,01756474365L,
	04723077174L,03642763134L,05750130273L,03655541561L,
};

CH_INTERN_FUNC( long )
dcalchash(datum item)
{
	register chint32 s, c, j;
	char* cp;
	register long hashl;
	register chint32 hashi; 

	hashl = 0;
	hashi = 0;
	for (cp = (pstr)item.dptr, s=item.dsize; --s >= 0; )
	{
		c = *cp++;
		for (j=0; j<BYTESIZ; j+=4)
		{
			hashi += hitab[c&017];
			hashl += hltab[hashi&63];
			c >>= 4;
		}
	}
	return (hashl);
}

/*
 * Delete pairs of items (n & n+1).
 */
CH_INTERN_FUNC( chint32 )
delitem(char buf[PBLKSIZ], long n)
{
	register short *sp, *sp1;
	register i1, i2;

	sp = (short *)buf;
	i2 = sp[0];
	if ((long)n >= i2 || (n & 1))
		return (0);

	if (n == i2-2)
	{
		sp[0] -= 2;
		return (1);
	}
	i1 = PBLKSIZ;
	if (n > 0)
		i1 = sp[n];
	i1 -= sp[n+2];
	if (i1 > 0)
	{
		i2 = sp[i2];
		ChMemMove( &buf[i2 + i1], &buf[i2], sp[n+2] - i2 );
		//bcopy(&buf[i2], &buf[i2 + i1], sp[n+2] - i2);
	}
	sp[0] -= 2;
	for (sp1 = sp + sp[0], sp += n+1; sp <= sp1; sp++)
		sp[0] = sp[2] + i1;

	return (1);
}

/*
 * Add pairs of items (item & item1).
 */
CH_INTERN_FUNC( chint32 )
additem(char buf[PBLKSIZ], datum item, datum item1)
{
	register short *sp;
	register chint32 i1, i2;

	sp = (short *)buf;
	i1 = PBLKSIZ;
	i2 = sp[0];
	if (i2 > 0)
		i1 = sp[i2];
	i1 -= item.dsize + item1.dsize;
	if (i1 <= (i2+3) * (chint32)sizeof(short))
		return (0);
	sp[0] += 2;
	sp[++i2] = (short)(i1 + item1.dsize);
	ChMemMove( &buf[i1 + item1.dsize], item.dptr, item.dsize );
	//bcopy(item.dptr, &buf[i1 + item1.dsize], item.dsize);
	sp[++i2] = (short )i1;
	ChMemMove( &buf[i1], item1.dptr, item1.dsize );
	//bcopy(item1.dptr, &buf[i1], item1.dsize);
	return (1);
}


CH_INTERN_FUNC( int )
ReadData( int iFile, long pos, void* pBuf, long lBufSize )
{
	#if defined( CH_MSW ) && defined( CH_ARCH_32 )
	// open mutex
	::WaitForSingleObject( hMutex, INFINITE );
	#endif

	// seek file
	(void) _lseek(iFile, pos, SEEK_SET );

	// read data   
	int iRead = read( iFile, pBuf, lBufSize);

	// close mutex

	#if defined( CH_MSW ) && defined( CH_ARCH_32 )
	::ReleaseMutex( hMutex );
	#endif


	return ( iRead );

}


CH_INTERN_FUNC( int )
WriteData( int iFile, long pos, void* pBuf, long lBufSize )
{
	// open mutex
	#if defined( CH_MSW ) && defined( CH_ARCH_32 )
	::WaitForSingleObject( hMutex, INFINITE );
	#endif

	// seek file
	(void) _lseek(iFile, pos, SEEK_SET );

	// read data   
	int iWrite = write( iFile, pBuf, lBufSize);

	// flush all the data
	_commit( iFile );


	// close mutex
	#if defined( CH_MSW ) && defined( CH_ARCH_32 )
	::ReleaseMutex( hMutex );
	#endif

	return iWrite;
}


#ifdef DEBUG
CH_INTERN_FUNC( chint32 )
chkblk(char buf[PBLKSIZ])
{
	register short *sp;
	register chuint16 t, i;

	sp = (short *)buf;
	t = PBLKSIZ;
	for (i=0; i<sp[0]; i++)
	{
		if (sp[i+1] > t)
			return (-1);
		t = sp[i+1];
	}
	if (t < (sp[0]+1)*sizeof(short))
		return (-1);
	return (0);
}
#endif
