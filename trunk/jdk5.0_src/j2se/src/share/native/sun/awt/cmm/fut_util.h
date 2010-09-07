/*
 * @(#)fut_util.h	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* fut_util.h	@(#)fut_util.h	1.3 10/23/98

 Fut (function table) internal interface file.  These definitions
 and macros are used only *internally* by the library and are
 considered private to it.  The header file is therefore not
 installed along with fut.h.  If something here needs to become
 public, please move it to fut.h rather than make this entire
 file public.

 COPYRIGHT (c) 1989-2003 Eastman Kodak Company.
 As  an  unpublished  work pursuant to Title 17 of the United
 States Code.  All rights reserved.
 */


#ifndef FUT_UTIL_HEADER
#define FUT_UTIL_HEADER

#include "fut.h"

/* functions referenced only internally by the library: */
#if defined(__cplusplus)
extern "C" {
#endif

KpInt32_t		fut_iomask_check (fut_p, KpInt32_t);
KpInt32_t		fut_required_inputs (fut_p, KpInt32_t);
KpInt32_t		fut_reset_iomask (fut_p);
KpInt32_t		fut_first_chan (KpInt32_t);
KpInt32_t		fut_mfutInfo (fut_p, KpInt32_p, KpInt32_p, KpInt32_p, KpInt32_t, KpInt32_p, KpInt32_p, KpInt32_p);

KpInt32_t		fut_unique_id (void);

fut_p			fut_alloc_fut (void);
fut_chan_p		fut_alloc_chan (void);
fut_itbl_p		fut_alloc_itbl (void);
fut_otbl_p		fut_alloc_otbl (void);
fut_gtbl_p		fut_alloc_gtbl (void);

fut_itbldat_p	fut_alloc_itbldat (fut_itbl_p);
fut_gtbldat_p	fut_alloc_gtbldat (fut_gtbl_p);
fut_otbldat_p	fut_alloc_otbldat (fut_otbl_p);

mf2_tbldat_p	fut_alloc_imftdat (fut_itbl_p, KpInt32_t);
mf2_tbldat_p	fut_alloc_gmftdat (fut_gtbl_p);
mf2_tbldat_p	fut_alloc_omftdat (fut_otbl_p, KpInt32_t);

KpChar_p		fut_alloc_idstr (KpInt32_t);
void			fut_free_idstr (KpChar_p);

KpInt32_t		fut_gtbl_imask (fut_gtbl_p);

#if defined(__cplusplus)
}
#endif

/* Magic numbers for runtime checking */
#define FUT_MAGIC	0x66757466	/* = "futf", (KpChar_t)FUT_MAGIC == 'f' */
#define FUT_CMAGIC	0x66757463	/* = "futc", (KpChar_t)FUT_CMAGIC == 'c' */
#define FUT_IMAGIC	0x66757469	/* = "futi", (KpChar_t)FUT_IMAGIC == 'i' */
#define FUT_OMAGIC	0x6675746f	/* = "futo", (KpChar_t)FUT_OMAGIC == 'o' */
#define FUT_GMAGIC	0x66757467	/* = "futg", (KpChar_t)FUT_GMAGIC == 'g' */

#define FUT_CIGAM	0x66747566	/* = "ftuf", byte-swapped for file I/O */
#define FUT_CIGAMI	0x69747566	/* = "ituf", byte-swapped for file I/O */
#define FUT_CIGAMO	0x6f747566	/* = "otuf", byte-swapped for file I/O */
#define FUT_CIGAMG	0x67747566	/* = "gtuf", byte-swapped for file I/O */

/* Quick in-line checks on validity of futs, chans, itbls, otbls and gtbls. */
#define IS_FUT(x)	((x) != NULL && (x)->magic == FUT_MAGIC)
#define IS_CHAN(x)	((x) != NULL && (x)->magic == FUT_CMAGIC)
#define IS_ITBL(x)	((x) != NULL && (x)->magic == FUT_IMAGIC)
#define IS_OTBL(x)	((x) != NULL && (x)->magic == FUT_OMAGIC)
#define IS_GTBL(x)	((x) != NULL && (x)->magic == FUT_GMAGIC)

/* IS_SHARED() checks if an input, output, or grid table is referenced by anyone else. */
#define IS_SHARED(tbl)	(((tbl) != NULL) && ((tbl)->ref != 0))

/* Sometimes it is inconvenient to repeatedly test for a null channel before
 * trying to access one of its members.  In many cases, a null channel may be
 * assumed to have a null imask and null tables.
 * FUT_CHAN_IMASK() conveniently returns a null imask for a null channel.
 * FUT_CHAN_[IGO]TBL() conveniently returns a null [igo]tbl for a null channel. */
#define FUT_CHAN_IMASK(chan)	((chan==FUT_NULL_CHAN) ? (KpInt32_t)0 : chan->imask)
#define FUT_CHAN_GTBL(chan)	((chan==FUT_NULL_CHAN) ? FUT_NULL_GTBL : chan->gtbl)
#define FUT_CHAN_OTBL(chan)	((chan==FUT_NULL_CHAN) ? FUT_NULL_OTBL : chan->otbl)
#define FUT_CHAN_ITBL(chan,i)	((chan==FUT_NULL_CHAN) ? FUT_NULL_ITBL : chan->itbl[i])

 /* FUT_DOUBLE_EVEN rounds a number up to a double even value.
 * This is used to pad the idstring (with newlines) so that the
 * tables in binary files are more human readable. */
#define FUT_DOUBLE_EVEN(x)	(((x)+3) & ~3)

/* format types for evaluation */
#define FMT_GENERAL     		(0)
#define FMT_QD					(1)
#define FMT_EQSTRIDES			(2)
#define FMT_BIGENDIAN24			(3)
#define FMT_LITTLEENDIAN24		(4)
#define FMT_BIGENDIAN32			(5)
#define FMT_LITTLEENDIAN32		(6)

/* diagnotstics (error messages) will be normally printed to stderr
 * unless QUIET is defined */
#define QUIET

#ifndef QUIET
#include <stdio.h>
#define DIAG(s,x)	(void)fprintf(stderr,s,x)
#else
#define DIAG(s,x)
#endif /* QUIET */


#endif /* FUT_UTIL_HEADER */
