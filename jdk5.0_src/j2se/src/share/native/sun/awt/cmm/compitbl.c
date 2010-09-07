/*
 * @(#)compitbl.c	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)compitbl.c	1.2 98/09/22

  COPYRIGHT (c) 1991-1998 Eastman Kodak Company.
  As an unpublished work pursuant to Title 17 of the United
  States Code.  All rights reserved.
*/

#include "attrib.h"
#include "fut.h"
#include "fut_util.h"

/* fut_comp_itbl composes a separable fut, fut0, with the input tables
 * of another fut, fut1.  A new fut is returned which shares grid and
 * output tables with fut1 but has new input tables.
 *
 *	result_fut(x,y,...) = fut1(fut0.x(x),fut0.y(y),...)
 *
 * Iomask usage:
 *	imask => specifies which input input tables of fut1 to change.
 *		A NULL imask indicates change all itbls for which a
 *		channel is defined in fut0.
 *	omask => specifies which output channels are created for the
 *		result fut.  A NULL omask defaults to the omask of fut1.
 *	pmask => specifies those output channels in fut0 to passed through
 *		to the result fut in the event they don't exist in fut1
 *		(channels passed through share all tables with fut0).
 *	order => if specified, indicates the interpolation order for
 *		 evaluating fut0 to produce the new input tables for
 *		 fut1.
 *	INPLACE may be set to perform the composition in place,
 *		replacing the existing itbls and returning a pointer
 *		to the original fut.
 */
fut_p
	fut_comp_itbl (fut_p fut1, fut_p fut0, KpInt32_t iomask)
{
KpUInt16_t	ramp12[FUT_INPTBL_ENT], lut[FUT_NICHAN][FUT_INPTBL_ENT], tmp;
KpUInt16_p	idata[FUT_NICHAN], luts[FUT_NICHAN];
KpInt32_t	i, j, imask, omask, pmask, in_place;
fut_p		new_fut;

	if ((! IS_FUT(fut1)) || (! IS_FUT(fut0))) {	/* check for valid fut1 and fut0 */
		return (FUT_NULL);
	}

	if (! fut_is_separable(fut0)) {	/* make sure fut0 is separable */
		return (FUT_NULL);
	}

	if (fut_to_mft (fut0) != 1) {	/* convert to mft for evaluation */
		return (FUT_NULL);
	}	

	if (mft_to_fut (fut1) != 1) {	/* for the short term make sure it's a fut */
		return (FUT_NULL);			/* but fut_comp_ilut should be changed to use mfts */
	}	

	/* unpack in_place bit. If zero, create a new fut which shares all of its tables */
	in_place = (KpInt32_t)FUT_IPMASK(iomask);
	if ( in_place ) {
		new_fut = fut1;
	}
	else {
		new_fut = fut_copy (fut1);
	}

	/* unpack output mask. If zero, use output mask of fut1 */
	omask = (KpInt32_t)FUT_OMASK(iomask);
	if (omask == 0) {
		omask = fut1->iomask.out;	/* use fut1 omask */
	}
	else {
		omask &= fut1->iomask.out;	/* restrict to fut1 outputs */
	}

	/* remove any output channels not specified in omask */
	for ( i=0; i<FUT_NOCHAN; i++) {
		if ( (FUT_BIT(i) & omask) != 0 ) continue;
		fut_free_chan (new_fut->chan[i]);
		new_fut->chan[i] = FUT_NULL_CHAN;
		new_fut->iomask.out &= ~FUT_BIT(i);
	}

	/* unpack input mask.  If zero, use ouput mask of fut0 */
	imask = (KpInt32_t)FUT_IMASK(iomask);
	if (imask == 0 ) {
		imask = fut0->iomask.out;	/* use fut0 omask */
	}
	else {
		imask &= fut0->iomask.out;	/* restrict to fut0 outputs */
	}

	/* modify pass-thru mask to restrict to channels existing in fut0 but absent from omask */
	pmask = (KpInt32_t)FUT_PMASK(iomask);
	pmask &= fut0->iomask.out;
	pmask &= ~omask;

	/* if imask is still zero then there is nothing to compose */
	if ( imask != 0 ) {
	
		for (i = 0; i < FUT_INPTBL_ENT; i++) {
			ramp12[i] = (KpUInt16_t)(i << 8);				/* make a ramp as the input data */
		}

		/* set up the array of input and output buffers for evaluateFut
		 * the output buffers are the 1D luts corresponding to each channel
		   of the separable fut
		 */
		for ( i=0; i<FUT_NICHAN; i++ ) {
			luts[i] = & lut[i][0];
			idata[i] = & ramp12[0];
		}

		/* evaluate each specified channel to create luts from fut0 */
		for ( i=0; i<FUT_NOCHAN; i++ ) {
			if ( (imask & FUT_BIT(i)) != 0 ) {
				if ( ! evaluateFut (fut0, FUT_BIT(i), KCM_USHORT, FUT_INPTBL_ENT,
						 (KpGenericPtr_t FAR*) idata, (KpGenericPtr_t FAR*) &luts[i])) {
					return (0);
				}
				else {
					for (j = 0; j < FUT_INPTBL_ENT; j++) {
						tmp = luts[i][j];
						if ((tmp & 0xfff0) != 0xfff0) {
							tmp += 0x7;		/* round */
						}
						tmp >>= 4;	/* convert to 12 bit data */
						luts[i][j] = tmp;
					}
				}
			}
		}

					/* compose luts with fut1 input tables */
		if ( fut_comp_ilut (new_fut, 
				FUT_IN(imask)|FUT_INPLACE|FUT_12BITS, 
				(void *)luts) == FUT_NULL) {
			if ( ! in_place ) {
				fut_free (new_fut);
			}
			return (FUT_NULL);
		}
	}

					/* add pass-thru channels */
	for ( i=0; i<FUT_NICHAN; i++ ) {
		if ( pmask & FUT_BIT(i)) {
			fut_chan_p	chan;

			chan = fut_share_chan (fut0->chan[i]);
			if ( chan == FUT_NULL_CHAN ) {
				if ( ! in_place ) {
					fut_free (new_fut);
				}
				return (FUT_NULL);
			}
			if ( ! fut_add_chan (new_fut, FUT_OUT(FUT_BIT(i)), chan) ) {
				if ( ! in_place ) {
					fut_free (new_fut);
				}
				fut_free_chan (chan);
				return (FUT_NULL);
			}
		}
	}

	return (new_fut);

}
