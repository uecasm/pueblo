/*
 * jutils.c
 *
 * Copyright (C) 1991-1994, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains miscellaneous utility routines needed for both
 * compression and decompression.
 * Note we prefix all global names with "j" to minimize conflicts with
 * a surrounding application.
 */

// $Header$

#include "headers.h"

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#include <ChJpegDecoder.h>


/*
 * Arithmetic utilities
 */

CH_GLOBAL_FUNC( long )
jdiv_round_up (long a, long b)
/* Compute a/b rounded up to next integer, ie, ceil(a/b) */
/* Assumes a >= 0, b > 0 */
{
  return (a + b - 1L) / b;
}


CH_GLOBAL_FUNC( long )
jround_up (long a, long b)
/* Compute a rounded up to next multiple of b, ie, ceil(a/b)*b */
/* Assumes a >= 0, b > 0 */
{
  a += b - 1L;
  return a - (a % b);
}


/* On normal machines we can apply MEMCOPY() and MEMZERO() to sample arrays
 * and coefficient-block arrays.  This won't work on 80x86 because the arrays
 * are FAR and we're assuming a small-pointer memory model.  However, some
 * DOS compilers provide far-pointer versions of memcpy() and memset() even
 * in the small-model libraries.  These will be used if USE_FMEM is defined.
 * Otherwise, the routines below do it the hard way.  (The performance cost
 * is not all that great, because these routines aren't very heavily used.)
 */

#define FMEMCOPY(dest,src,size)	ChMemCopy(dest,src,size)
#define FMEMZERO(target,size)	ChMemClear(target,size)


CH_GLOBAL_FUNC( void )
jcopy_sample_rows (JSAMPARRAY input_array, int source_row,
		   JSAMPARRAY output_array, int dest_row,
		   int num_rows, JDIMENSION num_cols)
/* Copy some rows of samples from one place to another.
 * num_rows rows are copied from input_array[source_row++]
 * to output_array[dest_row++]; these areas may overlap for duplication.
 * The source and destination arrays must be at least as wide as num_cols.
 */
{
  register JSAMPROW inptr, outptr;
  register size_t count = (size_t) (num_cols * SIZEOF(JSAMPLE));
  register int row;

  input_array += source_row;
  output_array += dest_row;

  for (row = num_rows; row > 0; row--) {
    inptr = *input_array++;
    outptr = *output_array++;
    FMEMCOPY(outptr, inptr, count);
  }
}


CH_GLOBAL_FUNC( void  )
jcopy_block_row (JBLOCKROW input_row, JBLOCKROW output_row,
		 JDIMENSION num_blocks)
/* Copy a row of coefficient blocks from one place to another. */
{
  FMEMCOPY(output_row, input_row, num_blocks * (DCTSIZE2 * SIZEOF(JCOEF)));
}


CH_GLOBAL_FUNC( void )
jzero_far (void FAR * target, size_t bytestozero)
/* Zero out a chunk of FAR memory. */
/* This might be sample-array data, block-array data, or alloc_medium data. */
{
  FMEMZERO(target, bytestozero);
}
