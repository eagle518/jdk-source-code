/*
 * @(#)util.c	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 	@(#)util.c	1.2 98/09/22

	Contains:	Utility routines for fut library.
			 This module defines global data arrays and structures.

	COPYRIGHT (c) 1991-1998 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */
 
#include <string.h>

#include "fut.h"		/* external interface file */
#include "fut_util.h"		/* internal interface file */

/* fut_iomask_check checks a fut_eval iomask for consistency with
 * a given fut, returning a (possibly) modified iomask or 0 if some
 * inconsistency was found.  Consistency checks are as follows:
 *
 *   1. channels specified in the "pass-through" mask are 'or'ed
 *	with the input mask.
 *
 *   2. the fut must provide outputs specified in the omask but
 *	not appearing the pmask.  Any missing fut output that appears
 *	in the pmask will be copied through from the input.
 *
 *   3. the user must supply all inputs required by the fut to produce
 *	the outputs specified by the user.
 */
KpInt32_t
	fut_iomask_check (fut_p fut, KpInt32_t iomask)
{
	KpInt32_t	pmask = (KpInt32_t)FUT_PMASK(iomask);
	KpInt32_t	omask = (KpInt32_t)FUT_OMASK(iomask);
	KpInt32_t	imask = (KpInt32_t)FUT_IMASK(iomask);
	KpInt32_t	rmask;

	imask |= pmask;			/* user supplied inputs */

	/*
	 * required fut inputs are those inputs required for each channel
	 * specified in omask.  If any required inputs are missing,
	 * return ERROR.
	 */
	rmask = (KpInt32_t)fut_required_inputs (fut, omask);
	if ( rmask & ~imask )
		return (0);

	/*
	 * required fut outputs are those in omask which are not in
	 * pmask. If any required fut outputs are missing, return ERROR.
	 */
	rmask = omask & ~pmask;		/* required fut outputs */
	if ( rmask & ~fut->iomask.out )
		return (0);

	/*
	 * return modified iomask.
	 */
	return (iomask | FUT_IN(imask));
}


/* fut_required_inputs returns an input mask describing the required
 * inputs for the fut channels specified in omask.
 */
KpInt32_t
fut_required_inputs (fut_p fut, KpInt32_t omask)
{
	KpInt32_t	i;
	KpInt32_t	imask = 0;

	if ( fut == FUT_NULL )
		return (0);

	if ( ! IS_FUT (fut) )
		return (-1);

	if ( omask == 0 )
		omask = fut->iomask.out;

	for ( i=0; i<FUT_NOCHAN; i++ )
		if (omask & FUT_BIT(i))
			imask |= (KpInt32_t)FUT_CHAN_IMASK(fut->chan[i]);

	return (imask);
}


/* fut_first_chan returns the first channel number defined in a
 * channel mask or -1, if no channel is defined.
 */
KpInt32_t
fut_first_chan (KpInt32_t mask)
{
	KpInt32_t	i;

	if ( mask <= 0 )
		return (-1);

	for ( i=0; (mask & 1) == 0; mask >>= 1, i++ ) ;

	return (i);
}


/* fut_unique_id returns a unique id number for this currently running
 * application. Id numbers start at 1.  An id of zero is considered
 * non-unique elsewhere in the fut library.  Negative ids are reserved
 * to describe ramp tables.
 */
KpInt32_t
	fut_unique_id (void)
{
	static KpInt32_t	unique_id = 1;
	return (unique_id++);
}


/* fut_gtbl_imask computes the input mask for a given grid table
 * based on its dimensions.
 */
KpInt32_t
	fut_gtbl_imask (fut_gtbl_p gtbl)
{
KpInt32_t	i, imask = 0;

	if ( gtbl != 0 ) {
		for ( i=0; i<FUT_NICHAN; i++ ) {
			if ( gtbl->size[i] > 1 ) {
				imask |= FUT_BIT(i);
			}
		}
	}

	return (imask);
}


/* fut_reset_iomask recomputes the input and output mask portions
 * of a futs iomask, according to the existing input and grid tables.
 * it also performs a consistency check on the input and grid table
 * sizes, returning 0 (FALSE) if any problems, 1 (TRUE) if ok.
 */
KpInt32_t
	fut_reset_iomask (fut_p fut)
{
KpInt32_t		i, j;
					/* clear existing masks */
	fut->iomask.in = 0;
	fut->iomask.out = 0;

	for ( i=0; i<FUT_NOCHAN; i++ ) {
		fut_chan_p	chan = fut->chan[i];

		if ( chan == 0 ) continue;
					/* recompute masks */
		chan->imask = fut_gtbl_imask (chan->gtbl);
		fut->iomask.out |= FUT_BIT(i);
		fut->iomask.in |= chan->imask;

					/* consistency check */
		for ( j=0; j<FUT_NICHAN; j++ ) {
			if ( (chan->imask & FUT_BIT(j)) ) {
				if ( ! IS_ITBL(chan->itbl[j]) ||
				     chan->itbl[j]->size != chan->gtbl->size[j] )
					return (0);
			}
		}
	}

	return (1);
}


/* fut_is_separable returns 1 (TRUE) if each output channel of the fut
 * is a function only of the corresponding input, 0 (FALSE) otherwise.
 * (i.e. f(x,y,z) = (fx(x),fy(y),fz(z)) ).
 */
KpInt32_t
fut_is_separable (fut_p fut)
{
	KpInt32_t		i;
	fut_chan_p	chan;

	for ( i=0; i<FUT_NOCHAN; i++ ) {
		chan = fut->chan[i];
		if ( chan != 0 && chan->imask != (KpInt32_t)FUT_BIT(i) )
			return (0);
	}
	return (1);
}
