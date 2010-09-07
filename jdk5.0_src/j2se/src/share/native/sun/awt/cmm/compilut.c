/*
 * @(#)compilut.c	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)compilut.c	1.2 98/09/22
 
	COPYRIGHT (c) 1991-1998 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include "fut.h"
#include "fut_util.h"

static fut_itbl_p	fut_comp_itbl_ilut (fut_itbl_p, KpChar_p, KpInt32_t);
static KpInt32_t	fut_comp_chan_ilut (fut_chan_p, KpChar_p FAR*, fut_itbl_p FAR*, fut_itbl_p FAR*, KpInt32_t);
static mf2_tbldat_t	fut_itbl_interp(mf2_tbldat_p, KpInt16_t FAR);

/* fut_comp_ilut composes a set of 8 or 12-bit, 256 element look-up tables
 * with the input tables of a fut.  These may have been derived from
 * another separable fut.  A new fut is returned which shares grid and
 * output tables with the original but has new input tables.
 *
 * Iomask usage:
 *	imask => luts defined in vararglist to be composed with itbls.
 *		A NULL imask indicates one lut which is composed with
 *		the first defined input table.
 *	omask => unused
 *	pmask => unused
 *	INPLACE may be set to perform the composition in place,
 *		replacing the existing itbls and returning a pointer
 *		to the original fut.
 *	VARARGS may be used to specify an array of luts.
 *	12BITS  if set, supplied luts are 12-bit data (KpInt16_t).
 *		Otherwise, they are 8-bit (KpUInt8_t).
 */
fut_p
	fut_comp_ilut (	fut_p				fut,
					KpInt32_t			iomask,
					KpGenericPtr_t FAR*	srcluts)
{
KpChar_p	luts[FUT_NICHAN];
KpInt32_t	i, i2;
KpInt32_t	imask;
KpInt32_t	is_12bits;
KpInt32_t	in_place;
fut_p		new_fut;
fut_itbl_p	new_itbl;
fut_itbl_p	orig_itbls[FUT_NICHAN];

	if ( ! IS_FUT(fut) )
		return (FUT_NULL);

	/* unpack input mask.  If zero, use first defined channel */
	imask = (KpInt32_t)FUT_IMASK(iomask);
	if (imask == 0 ) {
		imask = (KpInt32_t)FUT_BIT(fut_first_chan((KpInt32_t)fut->iomask.in));
		iomask |= FUT_IN(imask);
	}

					/* get args specified by iomask */
	for ( i=0, i2 = 0; i<FUT_NICHAN; i++ ) {
		if (imask & FUT_BIT(i)) {
			luts[i] = srcluts[i2];	/* save lut address in array */
			i2++;
		}

	}

	/* if INPLACE is not set, create a new fut which shares all of its tables */
	in_place = (KpInt32_t)FUT_IPMASK(iomask);
	if ( in_place ) {
		new_fut = fut;
	}
	else {
		new_fut = fut_copy (fut);
	}

					/* unpack 12bit data flag */
	is_12bits = (KpInt32_t)FUT_12BMASK(iomask);

	/* for each lut passed, compose it with the specified input table(s) */
	/* start by composing the common itbls */
	for ( i=0; i<FUT_NICHAN; i++) {
		/* save original itbls for future comparison */
		orig_itbls[i] = fut->itbl[i];

		/* if no lut or no itbl, there's nothing to do. */
		if ( luts[i] == 0 || fut->itbl[i] == FUT_NULL_ITBL )
			continue;

		/* compose itbl with fut and replace the existing one. */
		new_itbl = fut_comp_itbl_ilut (fut->itbl[i], luts[i], is_12bits);
		if ( new_itbl == FUT_NULL_ITBL ) {
			if ( ! in_place )
				fut_free (new_fut);
			return (FUT_NULL);
		}
		fut_free_itbl (new_fut->itbl[i]);
		new_fut->itbl[i] = new_itbl;
	}

	/* now compose the itbls in each chan, re-sharing if possible */
	for ( i=0; i<FUT_NOCHAN; i++) {
		if ( new_fut->chan[i] == FUT_NULL_CHAN )
			continue;

		if ( ! fut_comp_chan_ilut (new_fut->chan[i], (KpChar_p FAR*)luts,
									orig_itbls, new_fut->itbl, is_12bits) ) {
			if ( ! in_place ) {
				fut_free (new_fut);
			}

			return (FUT_NULL);
		}
	}

	return (new_fut);
}


/* fut_comp_chan_ilut composes (in place) the input tables of a
 * fut output channel with the look-up tables specified in the
 * list, luts.  The original and new input tables from the
 * parent fut must be passed in order to preserve the sharing
 * of input tables.
 */
static KpInt32_t
	fut_comp_chan_ilut (fut_chan_p		chan,
						KpChar_p FAR*	luts,
						fut_itbl_p FAR*	orig_itbls,
						fut_itbl_p FAR*	new_itbls,
						KpInt32_t		is_12bits)
{
KpInt32_t	i;
fut_itbl_p	new_itbl;

	if ( ! IS_CHAN(chan) ) {
		return (0);
	}

	/* check each itbl to see if must be computed or shared */
	for ( i=0; i<FUT_NICHAN; i++ ) {
		if ((luts[i] == NULL) || (chan->itbl[i] == NULL))
			continue;

		if ((orig_itbls != NULL) && (chan->itbl[i] == orig_itbls[i])) {
			new_itbl = fut_share_itbl(new_itbls[i]);
		}
		else {
			new_itbl = fut_comp_itbl_ilut (chan->itbl[i], luts[i], is_12bits);
		}

		if (new_itbl == FUT_NULL_ITBL) {
			return (0);
		}

		/* replace with the new composed or shared itbl */
		fut_free_itbl (chan->itbl[i]);
		chan->itbl[i] = new_itbl;
	}

	return (1);
}


/* fut_comp_itbl_ilut composes an input table with an 8-bit or 12-bit,
 * 256 entry look-up table. It returns a newly allocated table or NULL
 * if an error occurred.  This is called by fut_comp_ilut() which in turn
 * is called by fut_comp_itbl().
 *
 * Although we are recomputing the input table here, we do so by simply
 * rearranging an existing one (in case of an 8-bit lut) or by linearly
 * interpolating within an existing one (in the case of a 12-bit lut).
 * Therefore, there is no need to do any clipping to avoid referencing
 * off-grid values - provided, of course, that the existing input table
 * has been clipped (see note in fut_calc_itbl()).
 */
static fut_itbl_p
	fut_comp_itbl_ilut (fut_itbl_p itbl, KpChar_p lut, KpInt32_t is_12bits)
{
fut_itbl_p		new_itbl;
mf2_tbldat_p	idat, new_idat;

	if ((itbl->dataClass != KCP_FIXED_RANGE) || (itbl->refTblEntries != FUT_INPTBL_ENT)) {
		return (FUT_NULL_ITBL);
	}

	/* create a new input table */
	new_itbl = fut_new_itblEx (KCP_REF_TABLES, KCP_FIXED_RANGE, itbl->size, FUT_NULL_IFUNEX, NULL);
	if ( new_itbl == FUT_NULL_ITBL )
		return (FUT_NULL_ITBL);

	/* assign a unique id since we are about to recompute */
	new_itbl->id = fut_unique_id ();

	/* reorder the input table entries using the lut */
	idat = itbl->refTbl;
	new_idat = new_itbl->refTbl;

	if ( is_12bits ) {
		/* since input tables are only 8 bits in, we must use the
		 * lowest 4 bits of the 12 bit data to interpolate between the
		 * input table entries specified by the highest 8 bits.
		 */
		KpInt16_p lut_p = (KpInt16_t FAR*) lut;
		KpInt16_p lut_end = lut_p + FUT_INPTBL_ENT;

		while ( lut_p < lut_end ) {
			*new_idat++ = fut_itbl_interp (idat, *lut_p);
			lut_p++;
		}
	}
	else {
		/* The lut is 8 bits so we simply do a table look up in the
		 * input table to find the new entries.
		 */
		KpUInt8_p lut_p = (KpUInt8_t FAR*) lut;
		KpUInt8_p lut_end = lut_p + FUT_INPTBL_ENT;

		while ( lut_p < lut_end ) {
			*new_idat++ = idat[*lut_p++];
		}
	}
	return (new_itbl);
}


/* 
 * Linearly interpolate an input table entry between itbl[i] and itbl[i+1]
 * using f as the interpolant, where i = integer part of 12-bit x and
 * f = fractional part.  t and p are (KpInt32_t) and (KpInt32_t *) scratch variables
 * In this version, the second parameter is 16 bits.
 */
static mf2_tbldat_t
	fut_itbl_interp (mf2_tbldat_p itbl, KpInt16_t x)											
{
	KpInt32_t result,temp1, temp2;
	mf2_tbldat_t t;
	mf2_tbldat_p p;
	
	
	p = &itbl[FUT_OTBL_INTEG(x)];
	t = *p++;
	
	temp1 = (*p-t) * FUT_OTBL_FRAC(x);
	temp2 = temp1 + FUT_OTBL_ROUNDUP;

	/* avoid right shifting a negative number (undefined result) */
	if (temp2 < 0) {
		result = t - (KpInt32_t)((-temp1 + FUT_OTBL_ROUNDUP - 1) >> FUT_OUT_FRACBITS);
	}
	else {
		result = t + (KpInt32_t)(temp2 >> FUT_OUT_FRACBITS) ;
	}
	return ((mf2_tbldat_t)result);
}
