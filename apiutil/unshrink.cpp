#include "headers.h"
#include <ChUnzip.h>

// $Header$

/*---------------------------------------------------------------------------

  unshrink.c                     version 0.94                     26 Apr 94

  Shrinking is basically a dynamic LZW algorithm with allowed code sizes of
  up to 13 bits; in addition, there is provision for partial clearing of
  leaf nodes.  PKWARE uses the special code 256 (decimal) to indicate a
  change in code size or a partial clear of the code tree:  256,1 for the
  former and 256,2 for the latter.  See the notes in the code below about
  orphaned nodes after partial clearing.

  This replacement version of unshrink.c was written from scratch.  It is
  based only on the algorithms described in Mark Nelson's _The Data Compres-
  sion Book_ and in Terry Welch's original paper in the June 1984 issue of
  IEEE _Computer_; no existing source code, including any in Nelson's book,
  was used.

  Memory requirements are fairly large.  While the NODE struct could be mod-
  ified to fit in a single 64KB segment (as a "far" data structure), for now
  it is assumed that a flat, 32-bit address space is available.  outbuf2 is
  always malloc'd, and flush() is always called with unshrink == FALSE.

  Copyright (C) 1994 Greg Roelofs.  See the accompanying file "COPYING" in
  the UnZip 5.11 (or later) source distribution.

  ---------------------------------------------------------------------------*/


/* #include "unzip.h" */

#if 0
#ifdef DEBUG
#  define OUTDBG(c)  if ((c)=='\n') {PUTC('^',stderr); PUTC('J',stderr);}\
                     else PUTC((c),stderr);
#else
#  define OUTDBG(c)
#endif
#else
#  define OUTDBG(c)
#endif
#if 0

typedef struct leaf {
    struct leaf *parent;
    struct leaf *next_sibling;
    struct leaf *first_child;
    uch value;
} NODE;

static void  partial_clear  __((NODE *cursib));

static NODE *node, *bogusnode, *lastfreenode;
#endif


int ChUnzip::unshrink()
{

	uch * stack = new uch[8192];
	
	if ( !stack )
	{
		return 	PK_MEM3;
	}

    uch *stacktop = stack + 8192 - 1;

    register uch *newstr;
    int codesize=9, code, oldcode=0, len, KwKwK;
    unsigned int outbufsiz;
	
	#if defined( CH_ARCH_16 )
    NODE _huge *freenode, _huge *curnode, _huge *lastnode=node, _huge *oldnode;
	#else
    NODE *freenode, *curnode, *lastnode=node, *oldnode;
	#endif


/*---------------------------------------------------------------------------
    Initialize various variables.
  ---------------------------------------------------------------------------*/

	#if defined( CH_ARCH_16 )
	HGLOBAL hMem = GlobalAlloc( GHND, (DWORD)(8192 * (DWORD)sizeof(NODE)) );
	if ( 0 ==  hMem )
	{
		delete []stack;
		return PK_MEM3;		
	}
	node = (NODE _huge*)GlobalLock( hMem );

	if ( 0 ==  node )
	{
		delete []stack;
		GlobalUnlock( hMem );
		GlobalFree( hMem );
		return PK_MEM3;		
	}
	#else
    if ((node = (NODE *)malloc(8192*sizeof(NODE))) == (NODE *)NULL)
	{
		delete []stack;
        return PK_MEM3;
	}
	#endif
    bogusnode = node + 256;
    lastfreenode = node + 256;


    /* this stuff was an attempt to debug compiler errors(?) when had
     * node[8192] in union work area...no clues what was wrong (SGI worked)
    Trace((stderr, "\nsizeof(NODE) = %d\n", sizeof(NODE)));
    Trace((stderr, "sizeof(node) = %d\n", sizeof(node)));
    Trace((stderr, "sizeof(area) = %d\n", sizeof(area)));
    Trace((stderr, "address of node[0] = %d\n", (int)&node[0]));
    Trace((stderr, "address of node[6945] = %d\n", (int)&node[6945]));
     */

    for (code = 0;  code < 256;  ++code) {
        node[code].value = code;
        node[code].parent = bogusnode;
        node[code].next_sibling = &node[code+1];
        node[code].first_child = (NODE *)NULL;
    }
    node[255].next_sibling = (NODE *)NULL;
    for (code = 257;  code < 8192;  ++code)
        node[code].parent = node[code].next_sibling = (NODE *)NULL;

	uch * outbuf;

	outbuf  = new uch[ OUTBUFSIZ + 1];	 /* extra: ASCIIZ */

	if ( !outbuf )
	{
		delete []stack;	

		#if defined( CH_ARCH_16 )
		GlobalUnlock( hMem );
		GlobalFree( hMem );
		#else		
    	free(node);
		#endif
    	return PK_MEM3;
	}

    uch *outptr = outbuf;
    int outcnt = 0L;
    if (pInfo->textmode)
        outbufsiz = RAWBUFSIZ;
    else
        outbufsiz = OUTBUFSIZ;

/*---------------------------------------------------------------------------
    Get and output first code, then loop over remaining ones.
  ---------------------------------------------------------------------------*/

    READBITS(codesize, oldcode)
    if (!zipeof) {
        *outptr++ = (uch)oldcode;
        OUTDBG((uch)oldcode)
        if (++outcnt == (int)outbufsiz) {
            flush(outbuf, outcnt, FALSE);
            outptr = outbuf;
            outcnt = 0L;
        }
    }

    do {
        READBITS(codesize, code)
        if (zipeof)
            break;
        if (code == 256) {   /* GRR:  possible to have consecutive escapes? */
            READBITS(codesize, code)
            if (code == 1) {
                ++codesize;
                TRACE1(" (codesize now %d bits)\n", codesize);
            } else if (code == 2) {
                TRACE( " (partial clear code)\n");
#if 0
#ifdef DEBUG
                fprintf(stderr, "   should clear:\n");
                for (curnode = node+257;  curnode < node+8192;  ++curnode)
                    if (!curnode->first_child)
                        fprintf(stderr, "%d\n", curnode-node);
                fprintf(stderr, "   did clear:\n");
#endif
#endif
                partial_clear(node);       /* recursive clear of leafs */
                lastfreenode = bogusnode;  /* reset start of free-node search */
            }
            continue;
        }

    /*-----------------------------------------------------------------------
        Translate code:  traverse tree from leaf back to root.
      -----------------------------------------------------------------------*/

        curnode = &node[code];
        newstr = stacktop;

        if (curnode->parent)
            KwKwK = FALSE;
        else {
            KwKwK = TRUE;
            TRACE2(" (found a KwKwK code %d; oldcode = %d)\n", code,
              oldcode);
            --newstr;   /* last character will be same as first character */
            curnode = &node[oldcode];
        }

        do {
            *newstr-- = curnode->value;
            curnode = curnode->parent;
        } while (curnode != bogusnode);

        len = stacktop - newstr++;
        if (KwKwK)
            *stacktop = *newstr;

    /*-----------------------------------------------------------------------
        Write expanded string in reverse order to output buffer.
      -----------------------------------------------------------------------*/

        //TRACE4( "code %4d; oldcode %4d; char %3d (%c); string [", code,
         // oldcode, (int)(*newstr), *newstr);
        {
            register uch *p;

            for (p = newstr;  p < newstr+len;  ++p) {
                *outptr++ = *p;
                OUTDBG(*p)
                if (++outcnt == (int)outbufsiz) {
                    flush(outbuf, outcnt, FALSE);
                    outptr = outbuf;
                    outcnt = 0L;
                }
            }
        }

    /*-----------------------------------------------------------------------
        Add new leaf (first character of newstr) to tree as child of oldcode.
      -----------------------------------------------------------------------*/

        /* search for freenode */
        freenode = lastfreenode + 1;
        while (freenode->parent)       /* add if-test before loop for speed? */
            ++freenode;
        lastfreenode = freenode;
        TRACE1( "]; newcode %d\n", freenode-node);

        oldnode = &node[oldcode];
        if (!oldnode->first_child) {   /* no children yet:  add first one */
            if (!oldnode->parent) {
                /*
                 * oldnode is itself a free node:  the only way this can happen
                 * is if a partial clear occurred immediately after oldcode was
                 * received and therefore immediately before this step (adding
                 * freenode).  This is subtle:  even though the parent no longer
                 * exists, it is treated as if it does, and pointers are set as
                 * usual.  Thus freenode is an orphan, *but only until the tree
                 * fills up to the point where oldnode is reused*.  At that
                 * point the reborn oldnode "adopts" the orphaned node.  Such
                 * wacky guys at PKWARE...
                 *
                 * To mark this, we set oldnode->next_sibling to point at the
                 * bogus node (256) and then check for this in the freenode sec-
                 * tion just below.
                 */
                TRACE2( "  [%d's parent (%d) was just cleared]\n",
                  freenode-node, oldcode );
                oldnode->next_sibling = bogusnode;
            }
            oldnode->first_child = freenode;
        } else {
            curnode = oldnode->first_child;
            while (curnode) {          /* find last child in sibling chain */
                lastnode = curnode;
                curnode = curnode->next_sibling;
            }
            lastnode->next_sibling = freenode;
        }
        freenode->value = *newstr;
        freenode->parent = oldnode;
        if (freenode->next_sibling != bogusnode)  /* no adoptions today... */
            freenode->first_child = (NODE *)NULL;
        freenode->next_sibling = (NODE *)NULL;

        oldcode = code;
    } while (!zipeof);

/*---------------------------------------------------------------------------
    Flush any remaining data, free malloc'd space and return to sender...
  ---------------------------------------------------------------------------*/

    if (outcnt > 0L)
        flush(outbuf, outcnt, FALSE);
	
	delete [] stack;
	#if defined( CH_ARCH_16 )
	GlobalUnlock( hMem );
	GlobalFree( hMem );
	#else
    free(node);
	#endif
    return PK_OK;

} /* end function unshrink() */





void ChUnzip::partial_clear(NODE * cursib)   /* like, totally recursive, eh? */
   // NODE *cursib;
{
    NODE *lastsib=(NODE *)NULL;

    /* Loop over siblings, removing any without children; recurse on those
     * which do have children.  This hits even the orphans because they're
     * always adopted (parent node is reused) before tree becomes full and
     * needs clearing.
     */
    do {
        if (cursib->first_child) {
            partial_clear(cursib->first_child);
            lastsib = cursib;
        } else if ((cursib - node) > 256) {  /* no children (leaf):  clear it */
            TRACE1( "%d\n", cursib-node);
            if (!lastsib)
                cursib->parent->first_child = cursib->next_sibling;
            else
                lastsib->next_sibling = cursib->next_sibling;
            cursib->parent = (NODE *)NULL;
        }
        cursib = cursib->next_sibling;
    } while (cursib);
    return;
}



