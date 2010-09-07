/*
 * @(#)io_swab.c	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)io_swab.c	1.2 98/11/10

	This file contains functions to handle architecture dependent byte swapping.

	COPYRIGHT (c) 1991-1998 Eastman Kodak Company.
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */

#include "fut_util.h"

/* fut_swab_hdr and fut_swab_[iog]tbl swaps bytes in a fut_hdr_t and
 * fut_[iog]tbl_t structures to convert between DEC and IBM byte ordering.
 */
void
fut_swab_hdr (fut_hdr_p hdr)
{
	KpInt32_t            i;
	chan_hdr_p	chan;

	Kp_swab32 ((KpGenericPtr_t)&hdr->magic,     1);
	Kp_swab32 ((KpGenericPtr_t)&hdr->version,   1);
	Kp_swab32 ((KpGenericPtr_t)&hdr->idstr_len, 1);
	Kp_swab32 ((KpGenericPtr_t)&hdr->order,     1);
	Kp_swab32 ( (KpGenericPtr_t)hdr->icode,     FUT_NCHAN);

	for (i=0, chan=hdr->chan; i<FUT_NCHAN; ++i, chan++) {
		Kp_swab16 ( (KpGenericPtr_t)chan->size,  FUT_NCHAN);
		Kp_swab32 ( (KpGenericPtr_t)chan->icode, FUT_NCHAN);
		Kp_swab32 ((KpGenericPtr_t)&chan->ocode, 1);
		Kp_swab32 ((KpGenericPtr_t)&chan->gcode, 1);
	}

	Kp_swab32 ((KpGenericPtr_t)&hdr->more,      1);

} /* fut_swab_hdr */

void
fut_swab_itbl (fut_itbl_p itbl)
{
	Kp_swab32 ((KpGenericPtr_t)&itbl->magic, 1);
	Kp_swab32 ((KpGenericPtr_t)&itbl->ref,   1);
	Kp_swab32 ((KpGenericPtr_t)&itbl->id,    1);
	Kp_swab32 ((KpGenericPtr_t)&itbl->size,  1);
	Kp_swab32 ((KpGenericPtr_t) itbl->tbl,   FUT_INPTBL_ENT+1);

	/* Kp_swab32 ((KpGenericPtr_t)&itbl->tbl, 1);       Never do this! */

} /* fut_swab_itbl */

void
fut_swab_otbl (fut_otbl_p otbl)
{
	Kp_swab32 ((KpGenericPtr_t)&otbl->magic, 1);
	Kp_swab32 ((KpGenericPtr_t)&otbl->ref,   1);
	Kp_swab32 ((KpGenericPtr_t)&otbl->id,    1);
	Kp_swab16 ((KpGenericPtr_t) otbl->tbl,   FUT_OUTTBL_ENT);

	/* Kp_swab32 (&otbl->tbl, 1);  Never do this! */

} /* fut_swab_otbl */

void
fut_swab_gtbl (fut_gtbl_p gtbl)
{
	KpInt32_t   tbl_size = gtbl->tbl_size;

					/* If gtbl is currently byte reversed,
					   we must swap tbl_size to determine
					   size of grid table */
	if ( gtbl->magic == FUT_CIGAMG )
		Kp_swab32 ((KpGenericPtr_t)&tbl_size, 1);


	Kp_swab32 ((KpGenericPtr_t)&gtbl->magic,    1);
	Kp_swab32 ((KpGenericPtr_t)&gtbl->ref,      1);
	Kp_swab32 ((KpGenericPtr_t)&gtbl->id,       1);
	Kp_swab16 ((KpGenericPtr_t) gtbl->tbl,      tbl_size / (KpInt32_t)sizeof(KpInt16_t));
	Kp_swab32 ((KpGenericPtr_t)&gtbl->tbl_size, 1);
	Kp_swab16 ((KpGenericPtr_t) gtbl->size,     FUT_NCHAN);

	/* Kp_swab32 (&gtbl->tbl, 1);  Never do this! */

} /* fut_swab_gtbl */
