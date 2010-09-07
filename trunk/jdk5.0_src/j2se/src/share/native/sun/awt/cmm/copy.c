/*
 * @(#)copy.c	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)copy.c	1.3 98/09/24

	functions to copy futs and their components.  Note that these are
	physical (hard) copies.  If a virtual (soft) copy will suffice
	for itbls, otbls, and gtbls, you can use fut_share_[iog]tbl().

	COPYRIGHT (c) 1991-1998 Eastman Kodak Company.
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */

#include "fut.h"
#include "fut_util.h"	/* internal interface file */


/* fut_copy copies an existing fut.
 */
fut_p
	fut_copy (fut_p fut)
{
fut_p		new_fut;
KpInt32_t	i;
KpHandle_t	h;

	if ( ! IS_FUT(fut)) {
		return (0);
	}

				/* allocate basic fut_structure */
	new_fut = fut_alloc_fut ();
	if ( new_fut == FUT_NULL ) {
		return (FUT_NULL);
	}

				/* save handle before copying over old fut */
	h = new_fut->handle;

				/* copy over all data (including pointers */
	*new_fut = *fut;

				/* now copy back handle */
	new_fut->handle = h;

				/* copy id string */
	new_fut->idstr = 0;
/*	(void) fut_set_idstr (new_fut, fut->idstr); */

				/* copy input tables */
	for ( i=0; i<FUT_NICHAN; i++ ) {
		new_fut->itbl[i] = (IS_SHARED (fut->itbl[i])) ?
							fut_share_itbl (fut->itbl[i]) : fut_copy_itbl (fut->itbl[i]);
		new_fut->itblHandle[i] = getHandleFromPtr((KpGenericPtr_t)new_fut->itbl[i]);
	}

				/* copy output channels */
	for ( i=0; i<FUT_NOCHAN; i++ ) {
		new_fut->chan[i] = fut_copy_chan (fut->chan[i]);
		new_fut->chanHandle[i] = getHandleFromPtr((KpGenericPtr_t)new_fut->chan[i]);
	}

				/* now check that all copies were succesful */
	if ( new_fut->idstr == 0 && fut->idstr != 0 ) {
		goto ErrOut;
	}

	for ( i=0; i<FUT_NICHAN; i++ ) {
		if ( new_fut->itbl[i] == 0 && fut->itbl[i] != 0) {
			goto ErrOut;
		}
	}
	for ( i=0; i<FUT_NOCHAN; i++ ) {
		if ( new_fut->chan[i] == 0 && fut->chan[i] != 0) {
			goto ErrOut;
		}
	}

	for ( i=0; i<FUT_NMCHAN; i++ ) {	/* free extra reference tables */
		if (NULL != fut->mabInRefTblHandles[i])
		{
			new_fut->mabInTblEntries[i] = fut->mabInTblEntries[i];
			new_fut->mabInRefTbl[i] = (mf2_tbldat_p)
									allocBufferPtr (new_fut->mabInTblEntries[i] * sizeof (mf2_tbldat_t));
			KpMemCpy (new_fut->mabInRefTbl[i], fut->mabInRefTbl[i],
													new_fut->mabInTblEntries[i] * sizeof (mf2_tbldat_t));
			new_fut->mabInRefTblHandles[i] = getHandleFromPtr ((KpGenericPtr_t)new_fut->mabInRefTbl[i]);
		}

		if (NULL != fut->mabOutRefTblHandles[i])
		{
			new_fut->mabOutTblEntries[i] = fut->mabOutTblEntries[i];
			new_fut->mabOutRefTbl[i] = (mf2_tbldat_p)
									allocBufferPtr (new_fut->mabOutTblEntries[i] * sizeof (mf2_tbldat_t));
			KpMemCpy (new_fut->mabOutRefTbl[i], fut->mabOutRefTbl[i],
													new_fut->mabOutTblEntries[i] * sizeof (mf2_tbldat_t));
			new_fut->mabOutRefTblHandles[i] = getHandleFromPtr ((KpGenericPtr_t)new_fut->mabOutRefTbl[i]);
		}
	}

	return (new_fut);
	

ErrOut:
	fut_free (new_fut);
	return (FUT_NULL);
}


/* fut_copy_chan makes a copy of an existing fut_chan_t structure.
 */
fut_chan_p
	fut_copy_chan (fut_chan_p chan)
{
fut_chan_p	new_chan;
KpInt32_t		i;
KpHandle_t		h;

	if ( ! IS_CHAN(chan) ) {
		return (FUT_NULL_CHAN);
	}

	new_chan = fut_alloc_chan ();
	if ( new_chan == FUT_NULL_CHAN ) {
		return (FUT_NULL_CHAN);
	}

				/* save handle before copying over old fut */
	h = new_chan->handle;

				/* copy over to new structure */
	*new_chan = *chan;

				/* move handle back to new structure */
	new_chan->handle = h;

	/* copy itbls
	 * if an itbl is shared, share it with the itbl in the new fut
	 */
	for ( i=0; i<FUT_NICHAN; i++ ) {
		new_chan->itbl[i] = (IS_SHARED (chan->itbl[i])) ?
							fut_share_itbl (chan->itbl[i]) : fut_copy_itbl (chan->itbl[i]);
		new_chan->itblHandle[i] = getHandleFromPtr((KpGenericPtr_t)new_chan->itbl[i]);
	}

					/* copy gtbl */
	new_chan->gtbl = fut_copy_gtbl (chan->gtbl);
	new_chan->gtblHandle =  getHandleFromPtr((KpGenericPtr_t)new_chan->gtbl);

					/* copy otbl */
	new_chan->otbl = (IS_SHARED(chan->otbl)) ?
						fut_share_otbl (chan->otbl) : fut_copy_otbl (chan->otbl);
	new_chan->otblHandle = getHandleFromPtr((KpGenericPtr_t)new_chan->otbl);

					/* check for successful copies */
	for ( i=0; i<FUT_NICHAN; i++ ) {
		if ( new_chan->itbl[i] == 0 && chan->itbl[i] != 0 ) {
			goto ErrOut;
		}
	}

	if ( (new_chan->otbl == 0 && chan->otbl != 0) ||
	     (new_chan->gtbl == 0 && chan->gtbl != 0) ) {
		goto ErrOut;
	}

	return (new_chan);
	

ErrOut:
	fut_free_chan (new_chan);
	return (FUT_NULL_CHAN);
}


/* fut_copy_itbl makes an exact copy of an existing fut_itbl_t
 */
fut_itbl_p
	fut_copy_itbl (fut_itbl_p itbl)
{
fut_itbl_p		new_itbl;
KpHandle_t		h;

				/* check for valid itbl */
	if ( ! IS_ITBL(itbl) ) {
		return (FUT_NULL_ITBL);
	}

				/* allocate the new itbl structure */
	new_itbl = fut_alloc_itbl ();
	if ( new_itbl == FUT_NULL_ITBL ) {
		DIAG("fut_copy_itbl: can't alloc input table struct.\n", 0);
		return (FUT_NULL_ITBL);
	}

	h = new_itbl->handle;	/* save handle before copying over old itbl */

	*new_itbl = *itbl;		/* copy entire struct except reference count */

	new_itbl->handle = h;	/* copy back handle */
	new_itbl->ref = 0;		/* first reference */

	if (itbl->tbl != NULL) {	/* copy fixed itbl data */
		new_itbl->tbl = fut_alloc_itbldat (new_itbl);
		if ( new_itbl->tbl == NULL ) {
			DIAG("fut_copy_itbl: can't alloc input table array.\n", 0);
			goto ErrOut;
		}
		new_itbl->tblHandle = getHandleFromPtr((KpGenericPtr_t)new_itbl->tbl);

		/* copy the table entries */
		KpMemCpy (new_itbl->tbl, itbl->tbl, (FUT_INPTBL_ENT+1)*sizeof(fut_itbldat_t));
	}

	if (itbl->refTbl != NULL) {	/* copy reference itbl data */
		new_itbl->refTbl = fut_alloc_imftdat (new_itbl, new_itbl->refTblEntries);
		if (new_itbl->refTbl == NULL ) {
			DIAG("fut_copy_itbl: can't alloc input table array.\n", 0);
			goto ErrOut;
		}

		/* copy the table entries */
		KpMemCpy (new_itbl->refTbl, itbl->refTbl, new_itbl->refTblEntries * sizeof (mf2_tbldat_t));
	}

	return (new_itbl);
	

ErrOut:
	fut_free_itbl (new_itbl);
	return (FUT_NULL_ITBL);
}


/* fut_copy_gtbl makes an exact copy of an existing fut_gtbl_t
 */
fut_gtbl_p
	fut_copy_gtbl (fut_gtbl_p gtbl)
{
fut_gtbl_p		new_gtbl;
KpInt32_t		gsize;
KpHandle_t		h;

				/* check for valid gtbl */
	if ( ! IS_GTBL(gtbl) ) {
		return (FUT_NULL_GTBL);
	}

	new_gtbl = fut_alloc_gtbl ();	/* allocate the new gtbl structure */
	if ( new_gtbl == FUT_NULL_GTBL ) {
		DIAG("fut_copy_gtbl: can't alloc grid table struct.\n", 0);
		return (FUT_NULL_GTBL);
	}

	h = new_gtbl->handle;	/* save handle before copying over old gtbl */

	*new_gtbl = *gtbl;		/* copy entire struct except reference count */

	new_gtbl->handle = h;
	new_gtbl->ref = 0;		/* first reference */

	if (gtbl->tbl != NULL) {	/* copy fixed gtbl data */
		gsize = gtbl->tbl_size / (KpInt32_t)sizeof(fut_gtbldat_t);
		new_gtbl->tbl = fut_alloc_gtbldat (new_gtbl);
		if ( new_gtbl->tbl == NULL ) {
			DIAG("fut_copy_gtbl: can't alloc grid table array.\n", 0);
			goto ErrOut;
		}
		new_gtbl->tblHandle = getHandleFromPtr((KpGenericPtr_t)new_gtbl->tbl);

		/* copy the table entries */
		KpMemCpy (new_gtbl->tbl, gtbl->tbl, gtbl->tbl_size);
	}

	if (gtbl->refTbl != NULL) {	/* copy reference gtbl data */
		new_gtbl->refTbl = fut_alloc_gmftdat (new_gtbl);
		if (new_gtbl->refTbl == NULL ) {
			DIAG("fut_copy_gtbl: can't alloc ref grid table array.\n", 0);
			goto ErrOut;
		}

		/* copy the table entries */
		KpMemCpy (new_gtbl->refTbl, gtbl->refTbl, gtbl->tbl_size);
	}

	return (new_gtbl);
	

ErrOut:
	fut_free_gtbl (new_gtbl);
	return (FUT_NULL_GTBL);
}


/* fut_copy_otbl makes an exact copy of an existing fut_otbl_t
 */
fut_otbl_p
	fut_copy_otbl (fut_otbl_p otbl)
{
fut_otbl_p		new_otbl;
KpHandle_t		h;

	/* check for valid otbl */
	if ( ! IS_OTBL(otbl) ) {
		return (FUT_NULL_OTBL);
	}

	/* allocate the new otbl structure */
	new_otbl = fut_alloc_otbl ();
	if ( new_otbl == FUT_NULL_OTBL ) {
		DIAG("fut_copy_otbl: can't alloc output table struct.\n", 0);
		return (FUT_NULL_OTBL);
	}

	h = new_otbl->handle;	/* save handle before copying over old otbl */

	*new_otbl = *otbl;		/* copy entire struct except reference count */

	new_otbl->handle = h;
	new_otbl->ref = 0;		/* first reference */

	if (otbl->tbl != NULL) {	/* copy fixed otbl data */
		new_otbl->tbl = fut_alloc_otbldat (new_otbl);
		if ( new_otbl->tbl == NULL ) {
			DIAG("fut_copy_otbl: can't alloc output table array.\n", 0);
			goto ErrOut;
		}
		new_otbl->tblHandle = getHandleFromPtr((KpGenericPtr_t)new_otbl->tbl);

		/* copy the table entries */
		KpMemCpy (new_otbl->tbl, otbl->tbl, FUT_OUTTBL_ENT * sizeof (fut_otbldat_t));
	}

	if (otbl->refTbl != NULL) {	/* copy reference otbl data */
		new_otbl->refTbl = fut_alloc_omftdat (new_otbl, new_otbl->refTblEntries);
		if (new_otbl->refTbl == NULL ) {
			DIAG("fut_copy_otbl: can't alloc output table array.\n", 0);
			goto ErrOut;
		}

		/* copy the table entries */
		KpMemCpy (new_otbl->refTbl, otbl->refTbl, new_otbl->refTblEntries * sizeof (mf2_tbldat_t));
	}

	return (new_otbl);
	

ErrOut:
	fut_free_otbl (new_otbl);
	return (FUT_NULL_OTBL);
}
