/*
 * @(#)iotable.c	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)iotable.c	1.1 98/09/15

	functions for accessing fut tables.

  COPYRIGHT (c) 1991-1998 Eastman Kodak Company.
  As an unpublished work pursuant to Title 17 of the United
  States Code.  All rights reserved.
*/

#include "fut.h"
#include "fut_util.h"		/* internal interface file */

/* local procedures */
static KpInt32_t fut_get_itbldat (fut_itbl_p, fut_itbldat_p FAR*);
static KpInt32_t has_chan (fut_p, KpInt32_t);


/* get an input table of a fut 
 */
KpInt32_t
  fut_get_itbl (fut_p fut, KpInt32_t ochan, KpInt32_t ichan, fut_itbldat_p* itblDat)
{
KpInt32_t theReturn = -1;		/* assume the worst */

	if (ichan < FUT_NICHAN) {
		if (ochan == -1) {
			if ( IS_FUT(fut) ) {	/* defined? */
				theReturn = fut_get_itbldat (fut->itbl[ichan], itblDat);
			}
		} else {
			if ((theReturn = has_chan(fut, ochan)) == 1) {
				theReturn = fut_get_itbldat (fut->chan[ochan]->itbl[ichan], itblDat);
			}
		}
		fut->modNum++;	/* increase modification level */
	}
	return (theReturn);

} /* fut_get_itbl */


/* get the data pointer of an input table of a fut 
 */
static KpInt32_t
  fut_get_itbldat (fut_itbl_p itbl, fut_itbldat_p* itblDat)
{
KpInt32_t theReturn = -2;	/* assuem table does not exist */

	if ( IS_ITBL(itbl) ) {	/* defined? */
		if (itbl->id <= 0) {
			itbl->id = fut_unique_id();	/* assume table gets changed, give it a real id */
		}
		if (itbl->refTbl != NULL) {
			*itblDat = (fut_itbldat_p)itbl->refTbl;		/* return input table */
			theReturn = 1;
		} else if (itbl->tbl != NULL) {
			*itblDat = itbl->tbl;			/* return input table */
			theReturn = 1;
		} else {
			theReturn = -1;					/* assume the worst */
		}
	}
		
	return (theReturn);

} /* fut_get_itbldat */


/* get a grid table of a fut 
 */
KpInt32_t
  fut_get_gtbl (fut_p fut, KpInt32_t ochan, fut_gtbldat_p* gtblDat)
{
KpInt32_t theReturn;
fut_gtbl_p gtbl;

	if ((theReturn = has_chan(fut, ochan)) == 1) {
		gtbl = fut->chan[ochan]->gtbl;
		if (gtbl->id <= 0) {
			gtbl->id = fut_unique_id();	/* assume table gets changed, give it a real id */
		}
		if (gtbl->refTbl != NULL) {
			*gtblDat = (fut_gtbldat_p)gtbl->refTbl;		/* return grid table */
		} else if (gtbl->tbl != NULL) {
			*gtblDat = gtbl->tbl;			/* return grid table */
		} else {
			theReturn = -1;					/* assume the worst */
		}
	}
	
	fut->modNum++;	/* increase modification level */
		
	return (theReturn);
	
} /* fut_get_gtbl */



/* get an output table of a fut 
 */
KpInt32_t
  fut_get_otbl (fut_p fut, KpInt32_t ochan, fut_otbldat_p* otblDat)
{
KpInt32_t theReturn;
fut_otbl_p otbl;

	if ((theReturn = has_chan(fut, ochan)) == 1) {
		otbl = fut->chan[ochan]->otbl;
		if (otbl->id <= 0) {
			otbl->id = fut_unique_id();	/* assume table gets changed, give it a real id */
		}
		if (otbl->refTbl != NULL) {
			*otblDat = (fut_otbldat_p)otbl->refTbl;		/* return output table */
		} else if (otbl->tbl != NULL) {
			*otblDat = otbl->tbl;			/* return output table */
		} else {
			theReturn = -1;					/* assume the worst */
		}
	}
	
	fut->modNum++;	/* increase modification level */
		
	return (theReturn);
	
} /* fut_get_otbl */



/* return 1 if channel is present, -1 if it is not */
static KpInt32_t 
	has_chan (fut_p fut, KpInt32_t chan)
{
KpInt32_t theReturn = -1;

	if ( IS_FUT(fut) &&
			(chan >= 0) &&
			(chan < FUT_NOCHAN) &&
			IS_CHAN(fut->chan[chan]) ) {
		theReturn = 1;											/* output channel exists */
	}
	
	return (theReturn);
}
