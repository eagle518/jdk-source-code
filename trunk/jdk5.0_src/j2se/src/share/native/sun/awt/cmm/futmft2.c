/*
 * @(#)futmft2.c	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)futmft2.c	1.4 98/11/16

	Contains:	functions to convert between futs and matrix futs.

	Author:		George Pawle

	COPYRIGHT (c) 1998 Eastman Kodak Company.
	As an unpublished work pursuant to Title 17 of the United
	States Code. All rights reserved.
 */

#include "kcptmgr.h"
#include "fut_util.h"
#include "attrib.h"

#define FUT_MATRIX_ZERO (0x0)
#define FUT_MATRIX_ONE (0x10000)
#define GBUFFER_SIZE (MF1_TBL_ENT*2)


/* fut_to_mft creates matrix-fut data tables from the fut data tables.
 * it does not overwrite any existing matrix-fut data.
 * the existing fut data tables are freed.
 *
 * Returns: 
 * 1 on success
 * -1 on memory allocation error
 */

KpInt32_t
	fut_to_mft (fut_p	fut)
{
KpInt32_t	status;

	status = makeMftTblDat (fut);	/* make the matrix-fut data tables */
	
	if (status == 1) {
		fut_free_tbldat (fut);		/* free the fut data tables */
	}
	
	return status;
}


/* makeMft2TblDat creates matrix-fut data tables from the fut data tables.
 * it does not overwrite any existing matrix-fut data.
 * the existing fut data tables are retained
 *
 * Returns: 
 * 1 on success
 * -1 on memory allocation error
 */

KpInt32_t
	makeMftTblDat (	fut_p		fut)
{
KpInt32_t		status, i1, i2, gTblEntries, srcOTblMax;
PTDataMap_t		outputMap;
KpUInt32_t		gData;
fut_chan_p		chan;
fut_gtbl_p		gtbl;
fut_otbl_p		otbl;
fut_gtbldat_p	srcPtr;
mf2_tbldat_p	dstPtr;

	/* convert each input table to mft2 */
	for (i1 = 0; i1 < FUT_NICHAN; i1++) {
		status = makeMftiTblDat (fut->itbl[i1]);
		if (status != 1) {
			goto GetOut;
		}
	}

	/* convert the output channels */
	for (i1 = 0; i1 < FUT_NOCHAN; i1++) {
		chan = fut->chan[i1];

		if ( ! IS_CHAN(chan)) {				/* no chan? */
			continue;
		}

		/* convert each input table to mft2 */
		for (i2 = 0; i2 < FUT_NICHAN; i2++) {
			status = makeMftiTblDat (chan->itbl[i2]);
			if (status != 1) {
				goto GetOut;
			}
		}

		/* convert the grid table */
		gtbl = chan->gtbl;
		if ((IS_GTBL(gtbl)) && 			/* have gtbl? */
			(gtbl->refTbl == NULL)) {	/* already mft2 format? */

			if (gtbl->tbl != NULL) {		/* have fixed table? */
				if (fut_alloc_gmftdat (gtbl) == NULL) {
					goto ErrOutM1;
				}

				gTblEntries = gtbl->tbl_size / sizeof (fut_gtbldat_t);
				srcPtr = gtbl->tbl;
				dstPtr = gtbl->refTbl;

				for (i2 = 0; i2 < gTblEntries; i2++) {
					gData = (KpUInt32_t) srcPtr [i2];
					
					gData = ((gData * MF2_TBL_MAXVAL) + (FUT_GRD_MAXVAL >> 1)) / FUT_GRD_MAXVAL;

					dstPtr[i2] = (mf2_tbldat_t) gData;				/* store each grid table entry */
				}
			}
		}

		/* convert the output tables to mft2 */
		if (chan->otbl == NULL) { 		/* no otbl? */
			if ((chan->otbl = fut_new_otblEx (KCP_REF_TABLES, KCP_FIXED_RANGE, fut_orampEx, NULL)) == NULL) {	/* create a ramp otbl */
				goto ErrOutM1;
			}
		}	

		otbl = chan->otbl;
		if ((IS_OTBL(otbl)) && 			/* have otbl? */
			(otbl->refTbl == NULL)) {	/* already mft2 format? */

			if (otbl->tbl != NULL) {		/* have fixed table? */
				/* set up the number of output table entries */
				if (otbl->refTblEntries == 0) {
					otbl->refTblEntries = FUT_OUTTBL_ENT;
				}

				if (fut_alloc_omftdat (otbl, otbl->refTblEntries) == NULL) {
					goto ErrOutM1;
				}

				/* set up the output table mapping */
				if (otbl->dataClass == KCP_VARIABLE_RANGE) {
					outputMap = KCP_BASE_MAX_TO_REF16;
					srcOTblMax = FUT_OUT_MAXVAL;
				}
				else {
					outputMap = KCP_MAP_END_POINTS;
					srcOTblMax = FUT_MAX_PEL12;
				}

				convert1DTable (otbl->tbl, sizeof (fut_otbldat_t), FUT_OUTTBL_ENT, srcOTblMax,
								otbl->refTbl, sizeof (mf2_tbldat_t), otbl->refTblEntries, MF2_TBL_MAXVAL,
								KCP_MAP_END_POINTS, outputMap);
			}
		}
	}

	status = 1;		/* success */

GetOut:
	return status;


ErrOutM1:
	status = -1;	/* memory allocation failure */
	goto GetOut;
}

	
KpInt32_t
	makeMftiTblDat (fut_itbl_p	itbl)
{
KpInt32_t	status = 1, iTableMaxValue;
PTDataMap_t	inputMap;

	if ((IS_ITBL(itbl)) && 			/* have itbl? */
		(itbl->refTbl == NULL)) {	/* already mft2 format? */

		if (itbl->tbl != NULL) {		/* have fixed table? */
			if (itbl->refTblEntries == 0) {	/* set up the number of input table entries */
				if (itbl->dataClass == KCP_VARIABLE_RANGE) {
					itbl->refTblEntries = MF2_STD_ITBL_SIZE;
				}
				else {
					itbl->refTblEntries = FUT_INPTBL_ENT;
				}
			}

			/* set up the input table mapping */
			if (itbl->dataClass == KCP_VARIABLE_RANGE) {
				inputMap = KCP_BASE_MAX_TO_REF16;
			}
			else {
				inputMap = KCP_MAP_END_POINTS;
			}

			if (fut_alloc_imftdat (itbl, itbl->refTblEntries) == NULL) {
				goto ErrOutM1;
			}
			
			iTableMaxValue = ((itbl->size -1) << FUT_INP_FRACBITS) -1;		

			convert1DTable (itbl->tbl, sizeof (fut_itbldat_t), FUT_INPTBL_ENT, iTableMaxValue,
							itbl->refTbl, sizeof (mf2_tbldat_t), itbl->refTblEntries, MF2_TBL_MAXVAL,
							inputMap, KCP_MAP_END_POINTS);
		}
	}

GetOut:	
	return status;		/* success */


ErrOutM1:
	status = -1;		/* memory allocation failure */
	goto GetOut;
}


/* mft_to_fut creates fut data tables from the matrix-fut data tables.
 * it does not overwrite any existing fut data.
 * the existing matrix-fut data tables are freed.
 *
 * Returns: 
 * 1 on success
 * -1 on memory allocation error
 */

KpInt32_t
	mft_to_fut (fut_p	fut)
{
KpInt32_t	status;

	status = makeFutTblDat (fut);	/* make the fut data tables */
	
	if (status == 1) {
		fut_free_mftdat (fut);		/* free the matrix-fut data tables */
	}
	
	return status;
}


/* makeFutTblDat adds fixed fut tables to a matrix fut.
 *
 * Returns: 
 * 1 on success
 * -1 on memory allocation error
 */

KpInt32_t
	makeFutTblDat (	fut_p	fut)
{
KpInt32_t		status, i1, i2, gTblEntries, dstOTblMax;
PTDataMap_t		outputMap;
KpUInt32_t		gData;
fut_chan_p		chan;
fut_gtbl_p		gtbl;
fut_otbl_p		otbl;
mf2_tbldat_p	srcPtr;
fut_gtbldat_p	dstPtr;

	/* convert each input table to mft2 */
	for (i1 = 0; i1 < FUT_NICHAN; i1++) {
		status = makeFutiTblDat (fut->itbl[i1]);
		if (status != 1) {
			goto GetOut;
		}
	}

	/* convert the output channels */
	for (i1 = 0; i1 < FUT_NOCHAN; i1++) {
		chan = fut->chan[i1];

		if (chan == NULL) {				/* no chan? */
			continue;
		}

		/* convert each mft2 input table to fixed */
		for (i2 = 0; i2 < FUT_NICHAN; i2++) {
			status = makeFutiTblDat (chan->itbl[i2]);
			if (status != 1) {
				goto GetOut;
			}
		}

		/* convert the grid table */
		gtbl = chan->gtbl;
		if ((IS_GTBL(gtbl)) &&	 			/* have gtbl? */
			(gtbl->refTbl != NULL)) {		/* have mft2 format? */

			if (gtbl->tbl == NULL) {
				if ((gtbl->tbl = fut_alloc_gtbldat (gtbl)) == NULL) {
					goto ErrOutM1;
				}
			}

			srcPtr = gtbl->refTbl;
			dstPtr = gtbl->tbl;
			gTblEntries = gtbl->tbl_size / sizeof (fut_gtbldat_t);

			for (i2 = 0; i2 < gTblEntries; i2++) {
				gData = (KpUInt32_t) srcPtr [i2];
				
				gData = ((gData * FUT_GRD_MAXVAL) + (MF2_TBL_MAXVAL >> 1)) / MF2_TBL_MAXVAL;

				dstPtr[i2] = (fut_gtbldat_t) gData;				/* store each grid table entry */
			}
		}

		/* convert the mft2 output tables to fixed */
		otbl = chan->otbl;
		if ((IS_OTBL(otbl)) && 			/* have otbl? */
			(otbl->refTbl != NULL)) {		/* have mft2 format? */

			if (otbl->tbl == NULL) {
				if ((otbl->tbl = fut_alloc_otbldat (otbl)) == NULL) {
					goto ErrOutM1;
				}
			}

			/* set up the output table mapping */
			if (otbl->dataClass == KCP_VARIABLE_RANGE) {
				outputMap = KCP_REF16_TO_BASE_MAX;
				dstOTblMax = FUT_OUT_MAXVAL;
			}
			else {
				outputMap = KCP_MAP_END_POINTS;
				dstOTblMax = FUT_MAX_PEL12;
			}

			convert1DTable (otbl->refTbl, sizeof (mf2_tbldat_t), otbl->refTblEntries, MF2_TBL_MAXVAL,
							otbl->tbl, sizeof (fut_otbldat_t), FUT_OUTTBL_ENT, dstOTblMax,
							KCP_MAP_END_POINTS, outputMap);
		}
	}

	status = 1;		/* success */

GetOut:
	return status;


ErrOutM1:
	status = -1;	/* memory allocation failure */
	goto GetOut;
}

	
KpInt32_t
	makeFutiTblDat (fut_itbl_p	itbl)
{
KpInt32_t	iTableMaxValue;
PTDataMap_t	inputMap;

	if ((IS_ITBL(itbl)) && 				/* have gtbl? */
		(itbl->refTbl != NULL)) {		/* have mft2 format? */
		
		/* set up the input table mapping */
		if (itbl->dataClass == KCP_VARIABLE_RANGE) {
			inputMap = KCP_REF16_TO_BASE_MAX;
		}
		else {
			inputMap = KCP_MAP_END_POINTS;
		}

		if (itbl->tbl == NULL) {
			if ((itbl->tbl = fut_alloc_itbldat (itbl)) == NULL) {
				goto ErrOutM1;
			}
		}
		
		iTableMaxValue = ((itbl->size -1) << FUT_INP_FRACBITS) -1;		

		convert1DTable (itbl->refTbl, sizeof (mf2_tbldat_t), itbl->refTblEntries, MF2_TBL_MAXVAL,
						itbl->tbl, sizeof (fut_itbldat_t), FUT_INPTBL_ENT, iTableMaxValue,
						inputMap, KCP_MAP_END_POINTS);

		itbl->tbl[FUT_INPTBL_ENT] = itbl->tbl[FUT_INPTBL_ENT -1];	/* dup last into last +1 */
	}

	return (1);		/* success */


ErrOutM1:
	return (-1);	/* memory allocation failure */
}
