/*
 * @(#)compose.c	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)compose.c	1.4 98/11/10

	COPYRIGHT (c) 1991-1998 Eastman Kodak Company.
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/


#include "attrib.h"
#include "kcptmgr.h"
#include "fut.h"
#include "fut_util.h"	/* internal interface file */

static KpInt32_t fut_comp_iotbl (fut_itbl_p, fut_otbl_p, fut_itbldat_p);
static KpInt32_t fut_comp_iotblMF (fut_itbl_p, fut_otbl_p, fut_itbl_p);


/* fut_comp composes one fut with another: fut2 = fut1(fut0).
 *
 * Omask specifies which channels are to be defined for the result fut.
 *	An empty omask indicates use omask of fut1.
 * Pmask specifies those channels of fut0 which are to be passed on to the
 *	result fut, in the event that fut1 cannot supply a channel stated
 *	in omask.
 * Order indicates the desired interpolation order to be performed in
 *	evaluating fut1.  (The result fut will have the same order as
 *	fut0, since their input grids are the same).
 */

fut_p
	fut_comp (fut_p fut1, fut_p fut0, KpInt32_t iomask)
{
KpInt32_t		ok = 1, nGridPoints, omask, evalomask, imask, pmask, order, i, j, nEntries, nOutChans;
fut_p			fut2 = NULL, evalFut = NULL;
fut_itbl_p		oitbls[FUT_NICHAN];
mf2_tbldat_p	indat[FUT_NICHAN], outdat[FUT_NOCHAN];
fut_gtbl_p		fut1_gtbls[FUT_NOCHAN];

	if (( ! IS_FUT(fut0)) || ( ! IS_FUT(fut1))) {
		return (NULL);
	}

	/* extract component masks from iomask */
	omask = FUT_OMASK(iomask);		/* which output chans? */
	pmask = FUT_PMASK(iomask);		/* which ones allowed to pass through? */
	order = FUT_ORDMASK(iomask);	/* which interpolation to use? */
	if ( order == FUT_DEFAULT ) {
		order = fut1->iomask.order;
	}

	/* adjust masks for iomask_check below */
	pmask &= fut0->iomask.out;		/* available for "pass through" */
	if ( omask == 0 ) {				/* required outputs (0 means all) */
		omask = fut1->iomask.out;
	}

	/* see if fut0 can provide required inputs to fut1 */
	imask = fut0->iomask.out;		/* available inputs for fut1 */
	iomask = FUT_OUT(omask) | FUT_IN(imask) | FUT_PASS(pmask);
	if ( ! fut_iomask_check (fut1, iomask) ) {
		return (NULL);
	}

	/* make sure the futs are in the reference state */
	if ((fut_to_mft (fut0) != 1) || (fut_to_mft (fut1) != 1)) {
		return (NULL);
	}	

	/* fut1 will be used to process the grid tables of fut0, placing the
	 * results in the grid tables of fut2.  Fut0's grid table data must first
	 * be passed through its output tables before sending it through fut1's
	 * input tables.  This is accomplished more efficiently by composing
	 * fut1's input tables with fut0's output tables and using these directly
	 * on fut0 grid data rather than the normal input tables.
	 *
	 * Create the result fut (fut2) which will be the composition of fut1
	 * and fut0.  Fut2 will inherit the input tables of fut0 and the output
	 * tables of fut1.  Its grid data will be in the same color coordinates
	 * as fut1's.
	 */
	fut2 = fut_new (FUT_IN(FUT_ALLIN), fut0->itbl, NULL, NULL);
	if ( fut2 == NULL ) {
		return (NULL);
	}

	/* for each desired channel i in fut2, create a new grid table.  The
	 * dimensions of each new grid table are derived from fut0 and fut1
	 * like so:  for every input required for channel i of fut1, form the
	 * union of the input sets of all corresponding fut0 outputs.
	 */

	/* null all io tables and table pointers */
	KpMemSet (oitbls, 0, sizeof(oitbls));

	imask = 0;			/* will be the input mask for all inputs needed to fut1 */
	evalomask = 0;		/* omask for evaluation */

	for (i = 0; (i < FUT_NOCHAN) && ok; i++) {
		KpInt32_t	size[FUT_NICHAN];
		fut_gtbl_p	gtbl;
		KpInt32_t	imask1, imask2;

		fut1_gtbls[i] = NULL;	/* assume not needed */

		if ((omask & FUT_BIT(i)) == 0) {	/* is this output channel needed? */
			continue;						/* no */
		}

		/* if a specified output is to be passed through from fut0, do that here */
		if ( ! IS_CHAN(fut1->chan[i]) && IS_CHAN(fut0->chan[i])) {

			ok = fut_defchan (fut2, FUT_OUT(FUT_BIT(i)), NULL,
							fut0->chan[i]->gtbl, fut0->chan[i]->otbl);

			continue;			/* no need to evaluate this ochan */
		}

		if (! IS_CHAN(fut1->chan[i])) {
			ok = 0;						/* something wrong */
			goto GetOut;
		}

		/* At this point we know that (fut1->chan[i] != 0).  We also
		 * have determined (from iomask_check above) that fut0->chan[j] != 0.
		 */
		imask2 = 0;						/* determine inputs from fut0 needed for this channel */
		imask1 = fut1->chan[i]->imask;	/* inputs used by this chan */

		for (j = 0; (j < FUT_NICHAN) && ok; j++) {
			if ((imask1 & FUT_BIT(j)) != 0) {		/* this input chan is needed */
				if ( ! IS_CHAN(fut0->chan[j])) {	/* available? */
					ok = 0;							/* composition fails */
					goto GetOut;
				}

				if (fut1->itbl[j] != fut1->chan[i]->itbl[j]) {	/* shared itbl? */
					goto nextOChan;								/* nope, ignore this ochan */
				}

				imask2 |= fut0->chan[j]->imask;
			}				
		}

		evalomask |= FUT_BIT(i);	/* will be evalutating this channel */
		imask |= imask1;			/* build mask of all needed inputs */
			
		/* determine required dimensions from mask */
		for (j = 0; j < FUT_NICHAN; j++) {
			size[j] = (imask2 & (KpInt32_t)FUT_BIT(j)) ? fut0->itbl[j]->size : 1;
		}

		/* create the new grid table
		 * insert it along with fut1's output table into fut2
		 */
		gtbl = fut_new_gtblEx (KCP_REF_TABLES, FUT_IN(FUT_ALLIN), NULL, NULL, size);
		ok = fut_defchan (fut2, FUT_OUT(FUT_BIT(i)), NULL, gtbl, fut1->chan[i]->otbl);
		fut_free_gtbl (gtbl);
		if (!ok) {
			goto GetOut;
		}

		fut1_gtbls[i] = fut1->chan[i]->gtbl;	/* collect gtbls for evaluation fut */

		/* verify the input data for the evaluation of the output channel in fut1 */
		for (j = 0; j < FUT_NICHAN; j++) {
			if ((imask1 & FUT_BIT(j)) != 0) {									/* this channel needed as input */
				if ((fut0->chan[j]->imask & (~fut2->chan[i]->imask)) != 0) {	/* it's inputs must be used by output */
					ok = 0;				/* composition fails */
					goto GetOut;
				}
			}
		}
nextOChan:;
	}

	/* collect the gtbls which are the input data for the chan evaluation.
	 * also pre-compose fut0's otbls with fut1's itbls.
	 */
	for (i = 0; i < FUT_NICHAN; i++) {
		oitbls[i] = NULL;

		if (ok) {
			fut_chan_p theChan = fut0->chan[i];
			
			if ((imask & FUT_BIT(i)) == 0) {
				continue;				/* this output from fut0 not required */
			}

			indat[i] = theChan->gtbl->refTbl;	/* collect gtbls: the input data for the evaluation */

			ok = (indat[i] != NULL);
			
			/* allocate memory for composed i/o tables
			 * these have the same size as the output tables of the channel supplying the input */
			if (ok) {
				fut_itbl_p	theITbl = fut1->itbl[i];
				fut_otbl_p	theOTbl = theChan->otbl;

				oitbls[i] = fut_alloc_itbl ();		/* get an itbl */
				if (NULL == oitbls[i])
				{
					ok = 0;							/* show we got an error (no memory) */
					goto GetOut;
				}
				oitbls[i]->size = theITbl->size;
				oitbls[i]->dataClass = KCP_FIXED_RANGE;

				nEntries = MAX(theITbl->refTblEntries, theOTbl->refTblEntries);

				ok = (fut_alloc_imftdat (oitbls[i], nEntries) != NULL);

				if (ok) {	/* make input table for evaluation */
					ok = fut_comp_iotblMF (theITbl, theOTbl, oitbls[i]);
				}
			}
		}
	}

	/* make an evaluation fut with the composed I/O tables, fut1's gtbls, and no otbls */
	evalFut = fut_new (iomask, oitbls, fut1_gtbls, NULL);
	if (( ! ok) ||
		(evalFut == NULL) ||		/* if evaluation fut ok */
		(fut_to_mft (fut2) != 1)) {	/* make sure the futs are in the reference state */
		ok = 0;
		goto GetOut;
	}	
	else {	/* Finally, we are ready to pass fut0's grid tables through fut1 */
		for (i = 0, nOutChans = 0; (i < FUT_NOCHAN) && ok; i++) {
			if ((evalomask & FUT_BIT(i)) != 0) {
				fut_gtbl_p	gtbl;
				
				gtbl = fut2->chan[i]->gtbl;
				nGridPoints = gtbl->tbl_size / sizeof (fut_gtbldat_t);	/* grid points for eval */

				if (evalFut->iomask.in != (unsigned int)evalFut->chan[i]->imask) {	/* must evaluate this channel singly */
					evalomask &= ~FUT_BIT(i);							/* remove channel from multiple eval list */
					ok = evaluateFut (evalFut, FUT_BIT(i), KCM_USHORT, nGridPoints,
									(KpGenericPtr_t FAR*) indat, (KpGenericPtr_t FAR*) &(gtbl->refTbl));
				}
				else {
					outdat[nOutChans] = gtbl->refTbl;
					nOutChans++;
				}

			}
		}

		/* eval result is composed fut's gtbls */
		ok = evaluateFut (evalFut, evalomask, KCM_USHORT, nGridPoints,
						 (KpGenericPtr_t FAR*) indat, (KpGenericPtr_t FAR*) outdat);
	}

GetOut:
	/* must always free up the evaluation fut and io tables, even if an error occurred! */
	fut_free (evalFut);
	fut_free_tbls (FUT_NICHAN, (void *)oitbls);

	/* check for errors */
	if ( !ok ) {
		fut_free (fut2);
		fut2 = NULL;
	}

	return (fut2);
}


/* fut_comp_iotblMF composes an output table with an input table.  The composite table
 * has the same data width and format as an input table, but has the number of entries of
 * an output table.
 */
static KpInt32_t
	fut_comp_iotblMF (	fut_itbl_p	itbl,
						fut_otbl_p	otbl,
						fut_itbl_p	dstitbl)
{
mf2_tbldat_t	x, intDestData, theOTbl[MF2_MAX_TBL_ENT];
mf2_tbldat_t	identOTbl[2] = {0, MF2_TBL_MAXVAL};
mf2_tbldat_p	srcOTblP, theOTblP;
fut_otbldat_p	optr;
KpInt32_t	i, tableIndex, tableIndexNext, srcOTblEntries;
KpFloat32_t	index, tableFrac, srcData, srcData1, destData, indexRatio;

	if ( ! IS_ITBL(itbl) || ! IS_OTBL(otbl) || ! IS_ITBL(dstitbl)) {
		return 0;
	}

	if (otbl->refTblEntries > dstitbl->refTblEntries) {
		return 0;	/* screwed up somewhere */
	}

	if ((srcOTblP = otbl->refTbl) == NULL) {
		srcOTblP = identOTbl;	/* null means identity */
		srcOTblEntries = 2;
	}
	else {
		srcOTblEntries = otbl->refTblEntries;
	}
	
	if (otbl->refTblEntries == dstitbl->refTblEntries) {
		theOTblP = otbl->refTbl;
	}
	else {	/* expand current otbl to specified size */
		theOTblP = theOTbl;

		convert1DTable (srcOTblP, sizeof (mf2_tbldat_t), srcOTblEntries, MF2_TBL_MAXVAL,
						theOTblP, sizeof (mf2_tbldat_t), dstitbl->refTblEntries, MF2_TBL_MAXVAL,
						KCP_MAP_END_POINTS, KCP_MAP_END_POINTS);
	}

	/* compose output table into this input table */
	optr = dstitbl->refTbl;
	indexRatio = (KpFloat32_t) (itbl->refTblEntries -1) / (KpFloat32_t) MF2_TBL_MAXVAL;

	for (i = 0; i < dstitbl->refTblEntries; i++) {
		x = theOTblP [i];	/* interpolation value */

		index = (KpFloat32_t) x * indexRatio;			/* calculate the input table position */
		tableIndex = (KpInt32_t) index;					/* the input table index */
		tableFrac = index - (KpFloat32_t) tableIndex;	/* and the input table interpolant */

		if (tableIndex >= itbl->refTblEntries) {	/* make sure we're in range for interpolation */
			tableIndex = itbl->refTblEntries -1;	/* 1st source is past end */
			tableIndexNext = tableIndex;
		}
		else {
			tableIndexNext = tableIndex +1;

			if (tableIndexNext == itbl->refTblEntries) {
				tableIndexNext = tableIndex;		/* 1st source is at end */
			}
		}

		srcData = (KpFloat32_t) itbl->refTbl [tableIndex];
		srcData1 = (KpFloat32_t) itbl->refTbl [tableIndexNext];

		destData = srcData + (tableFrac * (srcData1 - srcData));	/* interpolate */

		/* round and convert to integer */
		intDestData = (mf2_tbldat_t)(destData + 0.5);		
		if (intDestData > (mf2_tbldat_t)MF2_TBL_MAXVAL) {
			intDestData = MF2_TBL_MAXVAL;
		}

		optr [i] = intDestData;
	}

	return 1;
}
