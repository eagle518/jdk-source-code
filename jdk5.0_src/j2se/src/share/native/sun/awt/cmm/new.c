/*
 * @(#)new.c	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)new.c	1.3 99/01/08

	functions to create and destroy futs and their components.

	Author:	Kit Enscoe, George Pawle

	Note that when a fut, chan, or [iog]tbl is freed, its magic number
	is zeroed.	Since all fut functions test for the magic number (and
	return an error if invalid), this prevents the structures from being
	used in the event that a freed pointer is accidentally referenced.
	Also, because of this checking, there is no need to zero the memory
	handles! (or anything else).

	COPYRIGHT (c) 1989-2002 Eastman Kodak Company.
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include "fut.h"
#include "fut_util.h"		/* internal interface file */

#define FUT_MFTDATA (1 << 0)
#define FUT_FUTDATA (1 << 1)

/* local prototypes */
static void fut_free_chan_list_p (fut_chan_p FAR*, KpHandle_t FAR*);
static void fut_free_itbl_list_p (fut_itbl_p FAR*, KpHandle_t FAR*);
static void fut_free_otbl_p (fut_otbl_p, KpHandle_t);
static void fut_free_gtbl_p (fut_gtbl_p, KpHandle_t);
static void	fut_free_itbl_list	(fut_itbl_p FAR*);
static void fut_free_itbldat_list (fut_itbl_p FAR*, KpUInt32_t);


fut_p
	fut_free (fut_p	fut)
{
KpInt32_t	i;

	if ( ! IS_FUT(fut)) {			/* check if defined */
		return (fut);
	}

/*	fut_free_idstr (fut->idstr); */	/* free id string if exists */

	fut_free_itbl_list (fut->itbl);	/* free shared input tables */

	for ( i=0; i<FUT_NOCHAN; i++ ) {	/* free channels */
		fut_free_chan (fut->chan[i]);
		fut->chan[i] = FUT_NULL_CHAN;
	}

	for ( i=0; i<FUT_NMCHAN; i++ ) {	/* free extra reference tables */
		freeBuffer (fut->mabInRefTblHandles[i]);
		fut->mabInTblEntries[i] = 0;
		fut->mabInRefTbl[i]= NULL;
		fut->mabInRefTblHandles[i] = NULL;

		freeBuffer (fut->mabOutRefTblHandles[i]);
		fut->mabOutTblEntries[i] = 0;
		fut->mabOutRefTbl[i]= NULL;
		fut->mabOutRefTblHandles[i] = NULL;
	}
	
	fut->magic = 0;					/* free fut_t structure itself */
	freeBufferPtr ((KpGenericPtr_t)fut);

	return ((fut_p)NULL);
}


void
	fut_free_chan (fut_chan_p chan)
{
	if ( ! IS_CHAN(chan) )	/* check if defined */
		return;

	fut_free_itbl_list (chan->itbl);	/* free input tables */

	fut_free_otbl (chan->otbl);		/* free output table */

	fut_free_gtbl (chan->gtbl);		/* free grid table */

				/* free fut_chan_t structure itself */
	chan->magic = 0;
	freeBufferPtr ((KpGenericPtr_t)chan);

}


static void
	fut_free_itbl_list (fut_itbl_p	FAR * itbl_list)
{
KpInt32_t	i;

	if ( itbl_list == NULL ) {
		return;
	}

	for ( i=0; i<FUT_NICHAN; i++ ) {
		fut_free_itbl (itbl_list[i]);
		itbl_list[i] = NULL;
	}
}


static void
	fut_free_itbldat_list (fut_itbl_p	FAR* itbl_list, KpUInt32_t	mode)
{
KpInt32_t	i;

	if ( itbl_list == NULL ) {
		return;
	}

	for ( i=0; i<FUT_NICHAN; i++ ) {
		if ((mode & FUT_MFTDATA) != 0) {
			fut_free_imftdat (itbl_list[i], freeData);
		}
		
		if ((mode & FUT_FUTDATA) != 0) {
			fut_free_itbldat (itbl_list[i], freeData);
		}
	}
}


void
	fut_free_itbl (fut_itbl_p itbl)
{
	if ( ! IS_ITBL(itbl)) {		/* defined? */
		return;
	}
	
	if (itbl->ref != 0) {					/* last reference? */
		itbl->ref--;
	}
	else {
		fut_free_imftdat (itbl, freeTable);	/* free the data */
		fut_free_itbldat (itbl, freeTable);
		itbl->magic = 0;
		freeBufferPtr ((KpGenericPtr_t)itbl);
	}
}


void
	fut_free_gtbl (fut_gtbl_p gtbl)
{
	if ( ! IS_GTBL(gtbl)) {		/* defined? */
		return;
	}

	if (gtbl->ref != 0) {					/* last reference? */
		gtbl->ref--;
	}
	else {
		fut_free_gmftdat (gtbl, freeTable);	/* free the data */
		fut_free_gtbldat (gtbl, freeTable);
		gtbl->magic = 0;
		freeBufferPtr ((KpGenericPtr_t)gtbl);
	}
}


void
	fut_free_otbl (fut_otbl_p otbl)
{
	if ( ! IS_OTBL(otbl)) {		/* defined? */
		return;
	}

	if (otbl->ref != 0) {					/* last reference? */
		otbl->ref--;
	}
	else {
		fut_free_omftdat (otbl, freeTable);	/* free the data */
		fut_free_otbldat (otbl, freeTable);
		otbl->magic = 0;
		freeBufferPtr ((KpGenericPtr_t)otbl);
	}
}


void
	fut_free_tbldat		(fut_p	fut)
{
KpInt32_t	i;
fut_chan_p	chan;

	if (IS_FUT(fut)) {
		fut_free_itbldat_list (&fut->itbl[0], FUT_FUTDATA);
		
		for ( i=0; i<FUT_NOCHAN; i++ ) {
			chan = fut->chan[i];
			if (IS_CHAN(chan)) {
				fut_free_itbldat_list (&chan->itbl[0], FUT_FUTDATA);
				fut_free_gtbldat (chan->gtbl, freeData);
				fut_free_otbldat (chan->otbl, freeData);
			}
		}
	}
}


void
	fut_free_itbldat (	fut_itbl_p		itbl,
						fut_freeMode_t	mode)
{
	if ( IS_ITBL(itbl) ) {
		if ((mode == freeTable) ||
			((mode == freeData) && (itbl->refTbl != NULL))) {

			freeBuffer (itbl->tblHandle);
			itbl->tbl = NULL;
			itbl->tblHandle = NULL;
		}
	}
}


void
	fut_free_gtbldat (	fut_gtbl_p		gtbl,
						fut_freeMode_t	mode)
{
	if ( IS_GTBL(gtbl) ) {
		if ((mode == freeTable) ||
			((mode == freeData) && (gtbl->refTbl != NULL))) {

			freeBuffer (gtbl->tblHandle);
			gtbl->tbl = NULL;
			gtbl->tblHandle = NULL;
		}
	}
}


void
	fut_free_otbldat (	fut_otbl_p		otbl,
						fut_freeMode_t	mode)
{
	if ( IS_OTBL(otbl) ) {
		if ((mode == freeTable) ||
			((mode == freeData) && (otbl->refTbl != NULL))) {

			freeBuffer (otbl->tblHandle);
			otbl->tbl = NULL;
			otbl->tblHandle = NULL;
		}
	}
}


void
	fut_free_mftdat		(fut_p	fut)
{
KpInt32_t	i;
fut_chan_p	chan;

	if (IS_FUT(fut)) {
		fut_free_itbldat_list (&fut->itbl[0], FUT_MFTDATA);
		
		for ( i=0; i<FUT_NOCHAN; i++ ) {
			chan = fut->chan[i];
			if (IS_CHAN(chan)) {
				fut_free_itbldat_list (&chan->itbl[0], FUT_MFTDATA);
				fut_free_gmftdat (chan->gtbl, freeData);
				fut_free_omftdat (chan->otbl, freeData);
			}
		}
	}
}


void
	fut_free_imftdat (	fut_itbl_p		itbl,
						fut_freeMode_t	mode)
{
	if (IS_ITBL(itbl)) {
		if ((mode == freeTable) ||
			((mode == freeData) && (itbl->tbl != NULL))) {

			freeBuffer (itbl->refTblHandle);
			itbl->refTblEntries = 0;
			itbl->refTbl = NULL;
			itbl->refTblHandle = NULL;
		}
	}
}


void
	fut_free_gmftdat (	fut_gtbl_p		gtbl,
						fut_freeMode_t	mode)
{
	if (IS_GTBL(gtbl)) {
		if ((mode == freeTable) ||
			((mode == freeData) && (gtbl->tbl != NULL))) {

			freeBuffer (gtbl->refTblHandle);
			gtbl->refTbl = NULL;
			gtbl->refTblHandle = NULL;
		}
	}
}


void
	fut_free_omftdat (	fut_otbl_p		otbl,
						fut_freeMode_t	mode)
{
	if (IS_OTBL(otbl)) {
		if ((mode == freeTable) ||
			((mode == freeData) && (otbl->tbl != NULL))) {

			freeBuffer (otbl->refTblHandle);
			otbl->refTblEntries = 0;
			otbl->refTbl = NULL;
			otbl->refTblHandle = NULL;
		}
	}
}


/* fut_free_tbl frees any table regardless of type by checking the magic
 * number in the header.  It will also free a fut_t or a fut_chan_t.
 *
 * fut_free_tbls will free a null terminated list of any type of table,
 * useful for disposing of a set of tables which were used for constructing
 * a fut (which the fut has now absorbed and made shared copies of).
 */
void
	fut_free_tbl (KpGenericPtr_t tbl)
{
	/* Make sure that we do not have a NULL pointer */
	if( tbl == NULL ) {
		return;
	}

	switch (*(KpInt32_p)tbl) {
		case FUT_MAGIC:
		fut_free ((fut_p) tbl);
		break;

		case FUT_CMAGIC:
		fut_free_chan ((fut_chan_p) tbl);
		break;

		case FUT_IMAGIC:
		fut_free_itbl ((fut_itbl_p) tbl);
		break;

		case FUT_OMAGIC:
		fut_free_otbl ((fut_otbl_p) tbl);
		break;

		case FUT_GMAGIC:
		fut_free_gtbl ((fut_gtbl_p) tbl);
		break;
	}
}


void
	fut_free_tbls (	KpInt32_t			cnt,
					KpGenericPtr_t FAR* theTbls)
{
	for (; cnt != 0;) {
		fut_free_tbl(theTbls[--cnt]);
	}
}


/* Functions to free a fut but preserve the locked/unlocked
 * state of any element which is not actually freed.	Items
 * will not be freed if they are shared with another fut.
 */

fut_p
	fut_free_futH (KpHandle_t futHandle)
{
fut_p fut;
	
	fut = (fut_p) lockBuffer(futHandle);

	if (IS_FUT(fut)) {

					/* free id string if exists */
	/*	fut_free_idstr (fut->idstr); */

					/* free shared input tables */
		fut_free_itbl_list_p (fut->itbl, fut->itblHandle);

					/* free channels */
		fut_free_chan_list_p (fut->chan, fut->chanHandle);

					/* free fut_t structure itself */
		fut->magic = 0;
		freeBufferPtr ((KpGenericPtr_t)fut);
	}

	return ((fut_p)NULL);
}


/* fut_free_chan_list_p
		For each channel, This functions frees all input tables, 
		the output table and the grid table.	It then frees the
		actual fut_chan_t memory.
*/
static void
	fut_free_chan_list_p (	fut_chan_p FAR *	chan_list,
							KpHandle_t FAR *	chanHdl_list)
{
KpInt32_t	i;
fut_chan_p	chan;

	if ( (chan_list == NULL) || ( chanHdl_list == NULL)	)
		return;


	for ( i=0; i<FUT_NOCHAN; i++ ) {
		chan = chan_list[i];
		if (chan == NULL) {		/* chan is unlocked on entry */
			chan = lockBuffer(chanHdl_list[i]);
		}
		
		if (IS_CHAN(chan)) {
			fut_free_itbl_list_p (chan->itbl, chan->itblHandle);	/* free input tables */

			fut_free_otbl_p (chan->otbl, chan->otblHandle);		/* free output table */

			fut_free_gtbl_p (chan->gtbl, chan->gtblHandle);		/* free grid table */

			/* free fut_chan_t structure itself */
			chan->magic = 0;
			freeBufferPtr ((KpGenericPtr_t)chan);
			chan_list[i] = FUT_NULL_CHAN;
		}
	}
}


/* fut_free_itbl_list_p
		This function is passed a list of fut_itbl_t pointers
		and handles.	If the ref count is zero, the table and
		the fut_itbl_t is freed.	Otherwise the ref count is 
		decremented and the lock state of the fut_itbl_t
		is returned to it's state on entry.
*/
static void
	fut_free_itbl_list_p (	fut_itbl_p FAR *	itbl_list,
							KpHandle_t FAR *	itblHdl_list)
{
KpInt32_t	i;
fut_itbl_p	itbl;

	if ((itbl_list == NULL) || (itblHdl_list == NULL)) {
		return;
	}

	for (i = 0; i < FUT_NICHAN; i++) {
		itbl = itbl_list[i];
		if (itbl == NULL) {
			itbl = lockBuffer (itblHdl_list[i]);
		}

		if (IS_ITBL(itbl)) {
			if (itbl->ref == 0) {
				fut_free_itbl (itbl);	/* last reference being freed */
				itbl_list[i] = NULL;
				itblHdl_list[i] = NULL;
			}
			else {
				if ( itbl->ref > 0 ) {	/* still other references */
					itbl->ref--;
					if (itbl_list[i] == NULL) {	/* leave in original lock state */
						unlockBuffer(itblHdl_list[i]);
					}
				}
			}
		}
	}
}


/* fut_free_gtbl_p
		This function is passed a fut_gtbl_t pointer
		and handle.	If the ref count is zero, the table and
		the fut_gtbl_t is freed.  Otherwise the ref count is 
		decremented and the lock state of the fut_gtbl_t
		is returned to it's state on entry.
*/
static void
	fut_free_gtbl_p (	fut_gtbl_p	gtblP,
						KpHandle_t	gtblHdl)
{
fut_gtbl_p	gtbl = gtblP;

	if (gtblHdl == NULL) {
		return;
	}

	if (gtbl == NULL) {				/* gtbl is unlocked on entry */
		gtbl = lockBuffer(gtblHdl);
	}
	
	if (IS_GTBL(gtbl)) {
		if (gtbl->ref == 0) {
			fut_free_gtbl(gtbl);	/* last reference being freed */
		}
		else {
			if (gtbl->ref > 0) {	/* still other references, leave in original lock state */
				gtbl->ref--;
				if (gtblP == NULL) {
					unlockBuffer(gtblHdl);
				}
			}
		}
	}
}


/* fut_free_otbl_p
		This function is passed a fut_otbl_t pointer
		and handle.	If the ref count is zero, the table and
		the fut_otbl_t is freed.	Otherwise the ref count is 
		decremented and the lock state of the fut_otbl_t
		is returned to it's state on entry.
*/
static void
	fut_free_otbl_p (	fut_otbl_p	otblPtr,
						KpHandle_t	otblHdl)
{
fut_otbl_p	otbl = otblPtr;

	if (otblHdl == NULL) {
		return;
	}

	if (otbl == NULL) {		/* otbl is unlocked on entry */
		otbl = lockBuffer(otblHdl);
	}

	if (IS_OTBL(otbl)) {
		if (otbl->ref == 0) {	/* last reference being freed */
			freeBuffer(otbl->tblHandle);
			otbl->magic = 0;
			freeBufferPtr ((KpGenericPtr_t)otbl);
		}
		else {
			if (otbl->ref > 0) {	/* still other references */
				otbl->ref--;	/* leave in original lock state */
				if (otblPtr == NULL) {
					unlockBuffer(otblHdl);
				}
			}
		}
	}
}


/* fut_new allocates and initializes a new fut_t data structure.
 * iomask specifies which (common) input tables and which output channels
 * are being defined.	Additional channels may be added later using
 * fut_defchan.
 *
 * NOTES:
 *	1. All the tables must be packed into a single array.
 *
 *	2. If a needed input table is not supplied (as determined from the
 *	grid table) or if a supplied input table is NULL, then a ramp
 *	input table will be automatically generated and inserted into
 *	the common itbl list.	The grid sizes are inferred from the
 *	supplied grid tables.
 */
fut_p
	fut_new (	KpInt32_t		iomask,
				fut_itbl_p FAR*	itbls,
				fut_gtbl_p FAR*	gtbls,
				fut_otbl_p FAR*	otbls)
{
fut_itbl_p	itbl[FUT_NICHAN];
fut_otbl_p	otbl[FUT_NOCHAN];
fut_gtbl_p	gtbl[FUT_NOCHAN];
fut_p		fut;
KpInt32_t	tIndex, imask, omask, i;

					/* get input and output masks */
	imask = (KpInt32_t)FUT_IMASK(iomask);
	omask = (KpInt32_t)FUT_OMASK(iomask);
	if ( imask > FUT_ALLIN || omask > FUT_ALLOUT ) {
		DIAG("fut_new: too many input or output channels.\n", 0);
		return (NULL);
	}

	/* get args specified by iomask */
	for ( i=0, tIndex = 0; i<FUT_NICHAN; i++ ) {
		itbl[i] = (((imask & FUT_BIT(i)) != 0) && (itbls != NULL))
					? itbls[tIndex++] : FUT_NULL_ITBL;
	}
	for ( i=0, tIndex = 0; i<FUT_NOCHAN; i++ ) {
		gtbl[i] = FUT_NULL_GTBL;
		otbl[i] = FUT_NULL_OTBL;

		if ((omask & FUT_BIT(i)) != 0) {
			if (gtbls != NULL) {
				gtbl[i] = gtbls[tIndex];
			}

			if (otbls != NULL) {
				otbl[i] = otbls[tIndex];
			}

			tIndex++;
		}
	}

				/* allocate and clear the fut_t structure */
	fut = fut_alloc_fut ();
	if ( fut == NULL ) {
		return (NULL);
	}

				/* set the interpolation order */
	fut->iomask.order = (KpInt32_t)FUT_ORDMASK(iomask);

				/* insert the specified input tables */
	for ( i=0; i<FUT_NICHAN; i++ ) {
		if ( itbl[i] == NULL) continue;
		if ( ! IS_ITBL (itbl[i]) ) {
			fut_free (fut);
			return (NULL);
		}
		fut->iomask.in |= FUT_BIT(i);
		fut->itbl[i] = fut_share_itbl(itbl[i]);
		fut->itblHandle[i] = fut->itbl[i]->handle;
	}

				/* define the specified output channels */
	for ( i=0; i<FUT_NOCHAN; i++ ) {
		if ( gtbl[i] == NULL) continue;
		if ( ! fut_defchan(fut,FUT_OUT(FUT_BIT(i)),NULL,gtbl[i],otbl[i]) ) {
			fut_free (fut);
			return (NULL);
		}
	}
	fut->lutConfig = LUT_TYPE_UNKNOWN;
	return (fut);
}


/* fut_new_chan allocates and initializes a fut_chan_t data structure.
 * If a required input table is missing, a ramp of the proper grid size
 * will be created.	If a supplied itbl is not required, it will not be
 * inserted into the channel's private itbl list.	All tables which are
 * actually used are copied and so the caller is responsible for
 * freeing the passed tables if necessary.
 *
 * If VARARGS is used, the list of input tables may be relaced by a
 * single array of fut_itbl_t pointers.	This array must then be followed
 * by a fut_gtbl_p and a fut_otbl_p.
 */
fut_chan_p
	fut_new_chan (	KpInt32_t		iomask,
					fut_itbl_p FAR*	itbls,
					fut_gtbl_p		gtbl,
					fut_otbl_p		otbl)
{
fut_itbl_p	itbl[FUT_NCHAN];
fut_chan_p	chan;
KpInt32_t	imask, i, tIndex;

	/* get input mask */
	imask = (KpInt32_t)FUT_IMASK(iomask);

	/* get args specified by imask */
	for ( i=0, tIndex = 0; i<FUT_NCHAN; i++ ) {
		itbl[i] = ((imask & FUT_BIT(i)) && (itbls != NULL)) ? itbls[tIndex++] : NULL;
	}

				/* allocate and clear the fut_chan_t structure */
	chan = fut_alloc_chan ();
	if ( ! IS_CHAN(chan)) {
		return (NULL);
	}

				/* check for valid grid and output tables */
	if (( ! IS_GTBL(gtbl)) || ((otbl != NULL) && ( ! IS_OTBL(otbl))) ) {
		DIAG("fut_new_chan: invalid grid or output table.\n", 0);
		fut_free_chan (chan);
		return (NULL);
	}

	/* get required input channels from gtbl */
	chan->imask = fut_gtbl_imask(gtbl);

				/* insert the required input tables */
	for ( i=0; i<FUT_NICHAN; i++ ) {
		if ( (chan->imask & FUT_BIT(i)) == 0 ) continue;

		if ( itbl[i] == FUT_NULL_ITBL ) {
			chan->itbl[i] = fut_new_itblEx (KCP_REF_TABLES, KCP_FIXED_RANGE, gtbl->size[i], fut_irampEx, NULL);
			if ( chan->itbl[i] == NULL) {
				DIAG("fut_new_chan: can't create itbl.\n",0);
				fut_free_chan (chan);
				return (NULL);
			}

			chan->itblHandle[i] = chan->itbl[i]->handle;
		}
		else {
			if ( ! IS_ITBL (itbl[i])) {
				DIAG("fut_new_chan: invalid input table.\n", 0);
				fut_free_chan (chan);
				return (NULL);
			}
			else {
				if ( itbl[i]->size != gtbl->size[i] ) {
					DIAG("fut_new_chan: gtbl-itbl size mismatch.\n", 0);
					fut_free_chan (chan);
					return (NULL);
				}
				else {
					chan->itbl[i] = fut_share_itbl(itbl[i]);	/* share the input table */
					chan->itblHandle[i] = chan->itbl[i]->handle;
				}
			}
		}
	}

					/* insert grid and output tables */
	chan->gtbl = fut_share_gtbl (gtbl);
	chan->gtblHandle =	(IS_GTBL(chan->gtbl)) ? chan->gtbl->handle : FUT_NULL_HANDLE;
	
	if (IS_OTBL(otbl)) {
		chan->otbl = fut_share_otbl (otbl);
	}
	else {
		chan->otbl = fut_alloc_otbl();
	}

	chan->otblHandle = (IS_OTBL(chan->otbl)) ? chan->otbl->handle : FUT_NULL_HANDLE;

	return (chan);
}

/* fut_new_itbl creates a new input table for one dimension of a grid table
 * of size 'size'.	Ifun must be a pointer to a function accepting a double
 * and returning a double, both in the range (0.0,1.0).	A pointer to the
 * newly allocated table is returned.	(If ifun is NULL, table is not
 * initialized).
 */
fut_itbl_p
	fut_new_itblEx (	PTTableType_t	tableType,
						PTDataClass_t	iClass,
						KpInt32_t		size,
						fut_ifunc_t		ifun,
						fut_calcData_p	data)
{
fut_itbl_p	itbl;
KpInt32_t	nEntries;

	if ((size <= 1) || (size > FUT_GRD_MAXDIM)) {
		DIAG("fut_new_itbl: bad grid size (%d).\n", size);
		return (FUT_NULL_ITBL);
	}
					/* allocate input table structure */
	itbl = fut_alloc_itbl ();
	if ( ! IS_ITBL(itbl)) {
		DIAG("fut_new_itbl: can't alloc input table struct.\n", 0);
		return (FUT_NULL_ITBL);
	}

	itbl->size = size;
	itbl->dataClass = iClass;

					/* allocate the table */
	if (itbl->dataClass == KCP_VARIABLE_RANGE) {
		nEntries = MF2_STD_ITBL_SIZE;
	}
	else {
		nEntries = FUT_INPTBL_ENT;
	}

	if (tableType == KCP_PT_TABLES) {
		itbl->tbl = fut_alloc_itbldat (itbl);
		if ( itbl->tbl == NULL ) {
			DIAG("fut_new_itbl: can't alloc input table array.\n", 0);
			goto ErrOut;
		}
	} else {
		itbl->refTbl = fut_alloc_imftdat (itbl, nEntries);
		if ( itbl->refTbl == NULL ) {
			DIAG("fut_new_itbl: can't alloc input table array.\n", 0);
			goto ErrOut;
		}
	}

	/* compute the input table entries */
	if ( ! fut_calc_itblEx (itbl, ifun, data) ) {
		/* Note: fut_calc_itbl prints message on error */
		goto ErrOut;
	}

	return (itbl);


ErrOut:
	fut_free_itbl (itbl);
	return (FUT_NULL_ITBL);
}


/* fut_new_gtbl creates a new grid table and optionally intializes it.
 * The input channels defined for the grid are specified in the input
 * channel mask portion of iomask.	Each input defined must have a size
 * specified in a KpInt32_t array.
 * Gfun must be a pointer to a function accepting from zero to three
 * doubles (depending on values of sx, sy, and sz) in the range (0.0,1.0)
 * and returning a fut_gtbldat_t in the range (0,FUT_GRD_MAXVAL).
 * A pointer to the newly allocated table is returned if there were no
 * errors.	(If gfun is NULL, the table is not initialized).
 */
fut_gtbl_p
	fut_new_gtblEx (	PTTableType_t	tableType,
						KpInt32_t		iomask,
						fut_gfunc_t		gfun,
						fut_calcData_p	data,
						KpInt32_p		dimList)
{
fut_gtbl_p	gtbl;
KpInt32_t	imask, i, dim_size, grid_size;

					/* get input mask */
	imask = (KpInt32_t)FUT_IMASK(iomask);

					/* allocate grid table structure */
	gtbl = fut_alloc_gtbl ();
	if ( gtbl == FUT_NULL_GTBL ) {
		DIAG("fut_new_gtblA: can't alloc grid table struct.\n", 0);
		return (FUT_NULL_GTBL);
	}

	/* get sizes from dimList */
	grid_size = 1;
	for ( i=0; i<FUT_NCHAN; i++ ) {
		dim_size = (imask & FUT_BIT(i)) ? dimList[i] : 1;
		if ( dim_size <= 0 ) {
			dim_size = 1;		/* make sure > 0 */
		}
		gtbl->size[i] = (KpInt16_t)dim_size;
		grid_size *= (KpInt32_t)dim_size;
	}

					/* check for valid grid size */
	if ( grid_size <= 0 || grid_size > FUT_GRD_MAX_ENT ) {
		DIAG("fut_new_gtblA: bad grid table size (%d).\n", grid_size);
		fut_free_gtbl(gtbl);
		return (FUT_NULL_GTBL);
	}
	gtbl->tbl_size = (KpInt32_t)grid_size * (KpInt32_t)sizeof(fut_gtbldat_t);

					/* allocate grid table */
	if (tableType == KCP_PT_TABLES) {
		gtbl->refTbl = fut_alloc_gtbldat (gtbl);
	} else {
		gtbl->refTbl = fut_alloc_gmftdat (gtbl);
	}
	if ( gtbl->refTbl == NULL ) {
		DIAG("fut_new_gtblA: can't alloc grid table array.\n", 0);
		fut_free_gtbl(gtbl);
		return (FUT_NULL_GTBL);
	}

					/* compute the grid table entries */
	if ( ! fut_calc_gtblEx (gtbl, gfun, data) ) {
		fut_free_gtbl(gtbl);
		return (FUT_NULL_GTBL);
	}

	return (gtbl);
}


/* fut_new_otbl creates a new output table for one channel of a fut.
 * Ofun must be a pointer to a function accepting a fut_gtbldat_t in the
 * range (0,FUT_GRD_MAXVAL) and returning a fut_otbldat_t in the same
 * interval.	A pointer to the newly allocated table is returned.
 * (If ofun is NULL, table is not intialized!).
 */
fut_otbl_p
	fut_new_otblEx (	PTTableType_t	tableType,
						PTDataClass_t	oClass,
						fut_ofunc_t		ofun,
						fut_calcData_p	data)
{
fut_otbl_p	otbl;

					/* allocate output table structure */
	otbl = fut_alloc_otbl();
	if ( otbl == FUT_NULL_OTBL ) {
		DIAG("fut_new_otbl: can't alloc output table struct.\n", 0);
		return (FUT_NULL_OTBL);
	}

	otbl->dataClass = oClass;

					/* allocate the table */
	if (tableType == KCP_PT_TABLES) {
		otbl->tbl = fut_alloc_otbldat (otbl);
		if ( otbl->tbl == NULL ) {
			DIAG("fut_new_otbl: can't alloc output table array.\n", 0);
			fut_free_otbl (otbl);
			return (FUT_NULL_OTBL);
		}
	} else {
		otbl->refTbl = fut_alloc_omftdat (otbl, FUT_OUTTBL_ENT);
		if ( otbl->refTbl == NULL ) {
			DIAG("fut_new_otbl: can't alloc output table array.\n", 0);
			fut_free_otbl (otbl);
			return (FUT_NULL_OTBL);
		}
	}

					/* compute the output table entries */
	if ( ! fut_calc_otblEx (otbl, ofun, data) ) {
		fut_free_otbl (otbl);
		return (FUT_NULL_OTBL);
	}

	return (otbl);
}


/* create a new fut which has shared input tables and calculates the identity function */
fut_p
	fut_new_empty (	KpInt32_t		ndim,
					KpInt32_p		dim,
					KpInt32_t		nchan,
					PTDataClass_t	iClass,
					PTDataClass_t	oClass)
{
fut_p		fut;
KpInt32_t	iomask = 0, i1;

	if ((ndim > FUT_NICHAN) || (nchan > FUT_NOCHAN)) {
		return FUT_NULL;
	}
	
	for (i1 = 0; i1 < ndim; i1++) {
		iomask |= FUT_IN(FUT_BIT(i1));
	}

	for (i1 = 0; i1 < nchan; i1++) {
		iomask |= FUT_OUT(FUT_BIT(i1));
	}

	/* Compute shared input tables, grid tables, and output tables:  */
	fut = constructfut (iomask, dim, NULL, NULL, NULL, NULL, iClass, oClass);

	return (fut);
}


/* fut_defchan defines an output channel for a fut.	Returns FALSE(0) if
 * the output channel is already defined (or fut is NULL), TRUE(1)
 * otherwise.  The size of the grid table (if non-zero) must match those
 * of the corresponding input table.  If they do not, the channel remains
 * undefined and FALSE is returned.
 *
 * If a required input table is missing, the table will be shared
 * with the corresponding one from the list of common itbls.  If there
 * is no such table in the common list, a ramp table is created and
 * inserted into the common itbl list.
 *
 * Since fut_defchan is intended to be used for constructing futs with
 * shared input tables,	if an input table is supplied that conflicts with
 * a table in the common list, an error occurs.
 */
KpInt32_t
	fut_defchan (	fut_p			fut,
					KpInt32_t		iomask,
					fut_itbl_p FAR*	itbls,
					fut_gtbl_p		gtbl,
					fut_otbl_p		otbl)
{
fut_itbl_p	itbl[FUT_NICHAN];
fut_chan_p	chan;
KpInt32_t	imask, i, tIndex;

					/* check for valid fut */
	if ( ! IS_FUT(fut)) {
		return (0);
	}
					/* get input mask */
	imask = (KpInt32_t) FUT_IMASK(iomask);

					/* get args specified by imask */
	for ( i=0, tIndex = 0; i < FUT_NICHAN; i++ ) {
		if ((itbls != NULL) && ((imask & FUT_BIT(i)) != 0)) { 	/* if itbl is in arglist, use it */
			itbl[i] = (fut_itbl_p)itbls[tIndex++];
		}
		else {	/* use itbl from shared itbl list */
			itbl[i] = fut->itbl[i];
		}
	}

	chan = fut_new_chan ((KpInt32_t)(FUT_IN (FUT_ALLIN)), (fut_itbl_p FAR*)itbl, gtbl, otbl);
	if ( ! IS_CHAN(chan)) {
		return (0);
	}

	/* If fut_new_chan created a new itbl (ramp), add it to the
	 * common list.	However, if an itbl in the chan differs from
	 * one in the common list, return an error.
	 */
	for ( i=0; i < FUT_NICHAN; i++ ) {
		if ( chan->itbl[i] == NULL ) {
			continue;
		}
		
		if ( ! IS_ITBL(fut->itbl[i])) {
			fut->itbl[i] = fut_share_itbl(chan->itbl[i]);
			fut->itblHandle[i] = chan->itblHandle[i];
		}
		else {
			if ( fut->itbl[i] != chan->itbl[i] ) {
				DIAG("fut_defchan: conflicting itbls.\n", 0);
				fut_free_chan (chan);
				return (0);
			}
		}
	}

					/* insert channel into fut */
	if ( ! fut_add_chan (fut, iomask, chan) ) {
		fut_free_chan (chan);
		return (0);
	}

	return (1);
}


/* fut_add_chan inserts a new output channel into a fut.
 * Unlike itbls, otbls, and gtbls, the channel structure is not sharable
 * and so the caller must not free the chan after this call.	(If the
 * passed channel structure needs to be saved, use fut_copy_chan).
 * The iomask in this case simply tells which output channel is being
 * added, and if this channel already exists, an error (0) is returned.
 *
 * fut_add_chan is intended to be used in conjunction with fut_new_chan
 * to construct futs with independent input tables.	It does not update
 * the list of common input tables as does fut_new and fut_defchan and
 * should not be mixed with calls to fut_defchan.
 */
KpInt32_t
	fut_add_chan (fut_p	fut, KpInt32_t iomask, fut_chan_p chan)
{
	KpInt32_t		ochan;

	if ( ! IS_FUT(fut) || (chan != FUT_NULL_CHAN && ! IS_CHAN(chan)) ) {
		return (0);
	}

					/* get output channel no. */
	ochan = FUT_CHAN ((KpInt32_t)FUT_OMASK(iomask));

					/* prohibit redefinition of channel */
	if ( ochan >= FUT_NOCHAN || fut->chan[ochan] != NULL)
		return (0);
					/* insert channel into fut */
	fut->chan[ochan] = chan;
	fut->chanHandle[ochan] = (IS_CHAN(fut->chan[ochan])) ?
								fut->chan[ochan]->handle : FUT_NULL_HANDLE;

					/* update iomasks */
	if ( IS_CHAN(chan) ) {
		fut->iomask.out |= FUT_BIT(ochan);
		fut->iomask.in |= chan->imask;
	}

	return (1);
}
