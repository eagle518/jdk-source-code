/*
 * @(#)compotbl.c	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)compotbl.c	1.1 98/09/15
 
	COPYRIGHT (c) 1991-1998 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include "attrib.h"
#include "fut.h"
#include "fut_util.h"

/* fut_comp_otbl composes a separable fut, fut1, with the output tables
 * of another fut, fut0.  A new fut is returned which shares grid and
 * input tables with fut0 but has new input tables.
 *
 *	result_fut(x,y,...) =	fut1.x(fut0.x(x,y,...))
 *				fut1.y(fut0.y(x,y,...))
 *				 . . .
 *
 * Iomask usage:
 *	imask => specifies which output tables of fut0 to change.
 *		A NULL imask indicates change all otbls in fut0.
 *	omask => specifies which output channels are created for the
 *		result fut.  A NULL omask defaults to the omask of fut0.
 *	pmask => unused (output chans missing in fut1 are assumed to be
 *		identity functions.  Required output channels existing
 *		in fut0 are automatically passed through.)
 *	order => if specified, indicates the interpolation order for
 *		 evaluating fut1 to produce the new output tables for
 *		 fut0.
 *	INPLACE may be set to perform the composition in place,
 *		replacing the existing otbls and returning a pointer
 *		to the original fut0.
 */
fut_p
	fut_comp_otbl (fut_p fut1, fut_p fut0, KpInt32_t iomask)
{
KpGenericPtr_t	olut[FUT_NOCHAN];
KpInt32_t		i, imask, omask, order;
fut_p			new_fut;

	if ((! IS_FUT(fut1)) || (! IS_FUT(fut0))) {	/* check for valid fut1 and fut0 */
		return (FUT_NULL);
	}

	if (! fut_is_separable(fut1)) {	/* make sure fut1 is separable */
		return (FUT_NULL);
	}

	if (fut_to_mft (fut1) != 1) {	/* convert to mft for evaluation */
		return (FUT_NULL);
	}	

	/* create a new fut which shares all of fut0's tables */
	new_fut = fut_copy (fut0);
	if ( new_fut == FUT_NULL ) {
		return (FUT_NULL);
	}

	/* unpack output mask. If zero, use output mask of fut0 */
	omask = (KpInt32_t)FUT_OMASK(iomask);
	if (omask == 0) {
		omask = fut0->iomask.out;	/* use fut0 omask */
	}
	else {
		omask &= fut0->iomask.out;	/* restrict to fut0 outputs */
	}

	/* remove output channels in the new_fut which are not specified in omask */
	if ( new_fut->iomask.out & ~omask ) {
		for ( i=0; i<FUT_NOCHAN; i++) {
			if ( (FUT_BIT(i) & omask) != 0 ) continue;
			fut_free_chan (new_fut->chan[i]);
			new_fut->chan[i] = FUT_NULL_CHAN;
		}
		(void) fut_reset_iomask (new_fut);
	}

	/* unpack input mask.  If zero, use ouput mask of fut0 */
	imask = (KpInt32_t)FUT_IMASK(iomask);
	
	if (imask == 0 ) {
		imask = fut0->iomask.out;	/* use fut0 omask */
	}
	else {
		imask &= fut0->iomask.out;	/* restrict to fut0 outputs */
	}

	imask &= omask;				/* restrict to channels in omask */
	imask &= fut1->iomask.out;	/* restrict to chans defined in fut1 */

	/* for each channel in imask, create a new otbl and substitute it in new_fut */
	for ( i = 0; i < FUT_NOCHAN; i++ ) {
		fut_otbl_p	otbl;

		if ( (FUT_BIT(i) & imask) != 0 ) {
			if ( fut0->chan[i]->otbl == FUT_NULL_OTBL ) {	/* create a ramp */
				otbl = fut_new_otblEx (KCP_REF_TABLES, KCP_FIXED_RANGE, fut_orampEx, NULL);
			}
			else {						/* just copy existing */
				otbl = fut_copy_otbl (fut0->chan[i]->otbl);
			}

			if ( otbl == FUT_NULL_OTBL) {
					goto ErrOut;
			}

			/* assign a unique id since we are about to recompute */
			otbl->id = fut_unique_id ();

			/* replace otbl */
			fut_free_otbl (new_fut->chan[i]->otbl);
			new_fut->chan[i]->otbl = otbl;

			/* define inputs for evaluateFut */
			olut[i] = otbl->refTbl;
		}
	}

	/* use evaluateFut to evaluate fut1 using otbls of new_fut as input and output */
	order = (KpInt32_t)FUT_ORDMASK(iomask);
	iomask = FUT_IN(imask) | FUT_ORDER(order);

	/* evaluate each specified channel */
	for ( i = 0; i < FUT_NOCHAN; i++ ) {
		if ( (imask & FUT_BIT(i)) != 0 ) {		/* evaluate this channel */
			if ( ! evaluateFut (fut1, FUT_BIT(i), KCM_USHORT, FUT_OUTTBL_ENT,
					 (KpGenericPtr_t FAR*) &olut[i], (KpGenericPtr_t FAR*) &olut[i])) {
				goto ErrOut;
			}
		}
	}

	return (new_fut);


ErrOut:
	fut_free (new_fut);

	return (FUT_NULL);
}
