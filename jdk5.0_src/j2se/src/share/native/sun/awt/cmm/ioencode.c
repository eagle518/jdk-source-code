/*
 * @(#)ioencode.c	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)ioencode.c	1.3 98/11/10
 
  Contains:
	Functions to encode and decode a fut structure for i/o.
	Shared tables and ramps are encoded to or decoded from futio.
	This eliminates the transmission of redundant data. 

	COPYRIGHT (c) 1991-1998 Eastman Kodak Company.
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include <string.h>
#include "fut_util.h"

#define FUTIO_VERSION	'0'	/* must remain at this value */

static KpInt32_t	futio_encode_itbl (fut_itbl_p, fut_itbl_p FAR*, KpInt32_t);
static KpInt32_t	futio_encode_otbl (fut_otbl_p, fut_chan_p FAR*, KpInt32_t);
static KpInt32_t	futio_encode_gtbl (fut_gtbl_p, fut_chan_p FAR*, KpInt32_t);
static fut_itbl_p	futio_decode_itbl (KpInt32_t, fut_itbl_p, fut_itbl_p FAR*);
static fut_otbl_p	futio_decode_otbl (KpInt32_t, fut_otbl_p, fut_chan_p FAR*);
static fut_gtbl_p	futio_decode_gtbl (KpInt32_t, fut_gtbl_p, fut_chan_p FAR*);


/* fut_io_encode preprocess a fut structure for output.  It looks
 * for ramps and shared input and output tables and encodes them
 * in the futio structure.
 */
KpInt32_t
	fut_io_encode (	fut_p		fut,
					fut_hdr_p	futio)
{
KpInt32_t	i, j;

	if ( ! IS_FUT(fut)) {
		return 0;
	}
	
	KpMemSet (futio, 0, sizeof(fut_hdr_t));

						/* set hdr info */
	futio->magic = FUT_MAGIC;
	futio->version = FUTIO_VERSION;
	futio->order = fut->iomask.order;
	futio->idstr_len = (fut->idstr == 0) ? 0 : (KpInt32_t)strlen(fut->idstr) +1;

						/* encode the input tables */
	for ( i=0; i<FUT_NICHAN; i++) {
		futio->icode[i] = futio_encode_itbl (fut->itbl[i], fut->itbl, i);
	}


						/* encode each output channel */
	for ( i=0; i<FUT_NOCHAN; i++) {
		fut_chan_p chan = fut->chan[i];
		chan_hdr_p chanio = & futio->chan[i];

		if ( chan == 0 ) continue;

						/* for each grid input : */
		for ( j=0; j<FUT_NICHAN; j++ ) {
						/* save it's size */
			chanio->size[j] = chan->gtbl->size[j];
						/* encode it's input table */
			chanio->icode[j] = futio_encode_itbl (chan->itbl[j],
							     fut->itbl, j+1);
		}
						/* encode the output table */
		chanio->ocode = futio_encode_otbl ( chan->otbl, fut->chan, i);


						/* encode the grid table */
		chanio->gcode = futio_encode_gtbl ( chan->gtbl, fut->chan, i);
	}

	return (1);
}


/* futio_encode_itbl returns the table code for an input table.
 * It searches through a list of n itbls and returns FUTIO_SHARED if
 * a match is found.  Otherwise, it checks if the itbl is a ramp and
 * returns FUTIO_RAMP if so.  Otherwise, it returns FUTIO_UNIQUE.
 */
static
KpInt32_t
futio_encode_itbl (fut_itbl_p itbl, fut_itbl_p FAR * itbl_list,
					KpInt32_t n)
{
	KpInt32_t  j;

	if ( itbl == 0 )
		return (FUTIO_NULL);

	for ( j=0; j<n; j++, itbl_list++ ) {    /* search for identical table */
		if ( *itbl_list == 0 ) continue;
		if ( (*itbl_list)->id == itbl->id )
			break;
	}
	if ( j != n && itbl->id != 0 ) {        /* (id==0) means unique */

		return (FUTIO_SHARED | j);      /* found identical table */

	} else if ( itbl->id < 0 ) {

		return (FUTIO_RAMP | (-itbl->id));      /* is a ramp */

	} else {

		return (FUTIO_UNIQUE);          /* is unique */

	}
}


/* futio_encode_otbl returns the table code for an output table.
 * It searches through a list of n chans and returns FUTIO_SHARED if
 * a match is found.  Otherwise, it checks if the otbl is a ramp and
 * returns FUTIO_RAMP if so.  Otherwise, it returns FUTIO_UNIQUE.
 */
static
KpInt32_t
futio_encode_otbl (fut_otbl_p otbl, fut_chan_p FAR * chan_list,
					KpInt32_t n)
{
	KpInt32_t  j;

	if ( otbl == 0 )
		return (FUTIO_NULL);

	for ( j=0; j<n; j++, chan_list++ ) {    /* search for identical table */
		if ( *chan_list == 0 ) continue;
		if ( (*chan_list)->otbl == 0 ) continue;
		if ( (*chan_list)->otbl->id == otbl->id )
			break;
	}
	if ( j != n && otbl->id != 0 ) {        /* (id==0) means unique */

		return (FUTIO_SHARED | j);      /* found identical table */

	} else if ( otbl->id < 0 ) {

		return (FUTIO_RAMP | (-otbl->id));      /* is a ramp */

	} else {

		return (FUTIO_UNIQUE);          /* is unique */

	}
}


/* futio_encode_gtbl returns the table code for a grid table.
 * It searches through a list of n chans and returns FUTIO_SHARED if
 * a match is found.  Otherwise, it checks if the gtbl is a ramp and
 * returns FUTIO_RAMP if so.  Otherwise, it returns FUTIO_UNIQUE.
 */
static
KpInt32_t
futio_encode_gtbl (fut_gtbl_p gtbl, fut_chan_p FAR * chan_list,
					KpInt32_t n)
{
	KpInt32_t  j;

	if ( gtbl == 0 )
		return (FUTIO_NULL);

	for ( j=0; j<n; j++, chan_list++ ) {    /* search for identical table */
		if ( *chan_list == 0 ) continue;
		if ( (*chan_list)->gtbl == 0 ) continue;
		if ( (*chan_list)->gtbl->id == gtbl->id )
			break;
	}
	if ( j != n && gtbl->id != 0 ) {        /* (id==0) means unique */

		return (FUTIO_SHARED | j);      /* found identical table */

	} else if ( gtbl->id < 0 ) {

		return (FUTIO_RAMP | (-gtbl->id));      /* is a ramp */

	} else {

		return (FUTIO_UNIQUE);          /* is unique */

	}
}


/* fut_io_decode postprocess a fut structure after input.  It looks for
 * encoded tables in the futio structure, and produces corresponding
 * shared tables and ramps.  This function also recomputes the iomasks of
 * of fut based on the existence of the various tables, and assigns a unique
 * id number to each table.
 */
KpInt32_t
fut_io_decode (fut_p fut, fut_hdr_p futio)
{
	KpInt32_t    i, j;
						/* get hdr info */
	fut->iomask.order = (KpInt32_t)futio->order;

						/* decode the input tables */
	for ( i=0; i<FUT_NICHAN; i++ ) {
		fut->itbl[i] = futio_decode_itbl (futio->icode[i], fut->itbl[i], fut->itbl);
		if (fut->itbl[i] != FUT_NULL_ITBL ) {
			fut->itblHandle[i] = fut->itbl[i]->handle;
		}
	}


						/* decode each output channel */
	for ( i=0; i<FUT_NOCHAN; i++) {
		fut_chan_p chan = fut->chan[i];
		chan_hdr_p chanio = & futio->chan[i];

		if ( chan == 0 ) continue;

						/* for each grid input : */
		for ( j=0; j<FUT_NICHAN; j++ ) {
						/* decode it's input table */
			chan->itbl[j] = futio_decode_itbl (chanio->icode[j], chan->itbl[j], fut->itbl);
			if (chan->itbl[j] != FUT_NULL_ITBL ) {
				chan->itblHandle[j] = chan->itbl[j]->handle;
			}
		}

						/* decode the output table */
		chan->otbl = futio_decode_otbl (chanio->ocode, chan->otbl, fut->chan);
		if (chan->otbl != FUT_NULL_OTBL) {
			chan->otblHandle = chan->otbl->handle;
		}

						/* decode the grid table */
		chan->gtbl = futio_decode_gtbl (chanio->gcode, chan->gtbl, fut->chan);
		if (chan->gtbl != FUT_NULL_GTBL) {
			chan->gtblHandle = chan->gtbl->handle;
		}

	}

	if ( ! fut_reset_iomask (fut) ) {	/* recompute iomasks */
		return (0);
	}

	return (1);
}


/* futio_decode_itbl decodes an input table that was encoded with
 * futio_encode_itbl.  It either generates a ramp, shares a table found
 * in itbl_list, or assigns a unique id number to an existing table.
 * In the latter case, the reference count is zeroed since the table has
 * just been allocated for the first time.  Care must be taken not to
 * share this table until after this step.
 * It returns a pointer to the correct itbl in any case.
 */
static fut_itbl_p 
	futio_decode_itbl (	KpInt32_t		code,
						fut_itbl_p		itbl,
						fut_itbl_p FAR*	itbl_list)
{
	switch (code & FUTIO_CODE) {
	case FUTIO_NULL :
		break;

	case FUTIO_SHARED :
		itbl = fut_share_itbl(itbl_list[code & FUTIO_DATA]);
		break;

	case FUTIO_RAMP :
		itbl = fut_new_itblEx (KCP_REF_TABLES, KCP_FIXED_RANGE, (KpInt32_t)(code & FUTIO_DATA), fut_irampEx, NULL);
		break;

	case FUTIO_UNIQUE :
		itbl->id = fut_unique_id ();
		itbl->ref = 0;
		break;

	default :       /* error */
		fut_free_itbl (itbl);
		return (0);
	}
	return (itbl);
}


/* futio_decode_otbl decodes an output table that was encoded with
 * futio_encode_otbl.  It either generates a ramp, shares a table found
 * in chan_list, or assigns a unique id number to an existing table.
 * In the latter case, the reference count is zeroed since the table has
 * just been allocated for the first time.  Care must be taken not to
 * share this table until after this step.
 * It returns a pointer to the correct otbl in any case.
 */
static
fut_otbl_p 
futio_decode_otbl (KpInt32_t code, fut_otbl_p otbl,
					fut_chan_p FAR * chan_list)
{
	switch (code & FUTIO_CODE) {
	case FUTIO_NULL :
		break;

	case FUTIO_SHARED :
		otbl = fut_share_otbl(chan_list[code & FUTIO_DATA]->otbl);
		break;

	case FUTIO_RAMP :
		otbl = fut_new_otblEx (KCP_REF_TABLES, KCP_FIXED_RANGE, fut_orampEx, NULL);
		break;

	case FUTIO_UNIQUE :
		otbl->id = fut_unique_id ();
		otbl->ref = 0;
		break;

	default :       /* error */
		fut_free_otbl (otbl);
		return (0);
	}
	return (otbl);
}


/* futio_decode_gtbl decodes a grid that was encoded with
 * futio_encode_gtbl.  It either generates a ramp, shares a table found
 * in chan_list, or assigns a unique id number to an existing table.
 * In the latter case, the reference count is zeroed since the table has
 * just been allocated for the first time.  Care must be taken not to
 * share this table until after this step.
 * It returns a pointer to the correct gtbl in any case.
 */
static
fut_gtbl_p 
futio_decode_gtbl (KpInt32_t code, fut_gtbl_p gtbl,
					fut_chan_p FAR * chan_list)
{
	switch (code & FUTIO_CODE) {
	case FUTIO_NULL :
		break;

	case FUTIO_SHARED :
		gtbl = fut_share_gtbl(chan_list[code & FUTIO_DATA]->gtbl);
		break;

	case FUTIO_UNIQUE :
		gtbl->id = fut_unique_id ();
		gtbl->ref = 0;
		break;

	case FUTIO_RAMP :
	default :       /* error */
		fut_free_gtbl (gtbl);
		return (0);
	}
	return (gtbl);
}
