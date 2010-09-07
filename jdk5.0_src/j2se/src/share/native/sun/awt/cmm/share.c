/*
 * @(#)share.c	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)share.c	1.3 98/09/24

	COPYRIGHT (c) 1991-1998 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */

#include <string.h>
#include "fut.h"
#include "fut_util.h"

/* fut_share_chan returns a new fut_chan_t structure which shares all of
 * its tables with an existing chan_t.  While it would be possible to simply
 * increment the lock count, recovery from error would be much more difficult.
 */
fut_chan_p 
	fut_share_chan (fut_chan_p chan)
{
fut_chan_p	sh_chan;
KpInt32_t	i;
KpHandle_t	h;

	if ( ! IS_CHAN(chan) ) {
		return (FUT_NULL_CHAN);
	}

					/* alloc a new fut_chan_t */
	sh_chan = fut_alloc_chan ();
	if ( sh_chan == FUT_NULL_CHAN ) {
		return (FUT_NULL_CHAN);
	}

	h = sh_chan->handle;

					/* copy all header info */
	*sh_chan = *chan;

	sh_chan->handle = h;	/* restore handle */

					/* share input tables, handles for all tables are ok */
	for ( i=0; i<FUT_NICHAN; i++) {
		sh_chan->itbl[i] = fut_share_itbl (chan->itbl[i]);
		if ( (chan->itbl[i] != FUT_NULL_ITBL) && (sh_chan->itbl[i] == FUT_NULL_ITBL) ) {
			goto ErrOut;			/* sharing failed */
		}
	}

					/* share grid tables */
	sh_chan->gtbl = fut_share_gtbl (chan->gtbl);
	if ((chan->gtbl != FUT_NULL_GTBL ) && (sh_chan->gtbl == FUT_NULL_GTBL)) {
		goto ErrOut;			/* sharing failed */
	}

					/* share output tables */
	sh_chan->otbl = fut_share_otbl (chan->otbl);
	if ((chan->otbl != FUT_NULL_OTBL ) && (sh_chan->otbl == FUT_NULL_OTBL)) {
		goto ErrOut;			/* sharing failed */
	}

	return (sh_chan);


ErrOut:
	fut_free_chan (sh_chan);
	return (FUT_NULL_CHAN);
}


/* fut_share_itbl returns a new fut_itbl_t structure which shares all of
 * its tables with an existing fut_itbl_t.
 */
fut_itbl_p 
	fut_share_itbl (fut_itbl_p itbl)
{
	if ( ! IS_ITBL(itbl)) {
		return NULL;
	}

	if (itbl->ref >= 0) {
		itbl->ref++;
	}

	return itbl;
}


/* fut_share_gtbl returns a new fut_gtbl_t structure which shares all of
 * its tables with an existing fut_gtbl_t.
 */
fut_gtbl_p 
	fut_share_gtbl (fut_gtbl_p gtbl)
{
	if ( ! IS_GTBL(gtbl)) {
		return NULL;
	}

	if (gtbl->ref >= 0) {
		gtbl->ref++;
	}

	return gtbl;
}


/* fut_share_otbl returns a new fut_otbl_t structure which shares all of
 * its tables with an existing fut_otbl_t.
 */
fut_otbl_p 
	fut_share_otbl (fut_otbl_p otbl)
{
	if ( ! IS_OTBL(otbl)) {
		return NULL;
	}

	if (otbl->ref >= 0) {
		otbl->ref++;
	}

	return otbl;
}
