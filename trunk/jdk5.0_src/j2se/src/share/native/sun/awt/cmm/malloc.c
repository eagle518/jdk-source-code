/*
 * @(#)malloc.c	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)malloc.c	1.2 98/09/22

	Contains:	allocate and free, lock and unlock memory used for input, output, and grid tables

	COPYRIGHT (c) 1991-2003 Eastman Kodak Company.
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include "fut.h"
#include "fut_util.h"

static void			fut_lock_itbls	(fut_itbl_p FAR*, KpHandle_t FAR*);
static fut_chan_p	fut_lock_chan		(KpHandle_t);
static void			fut_unlock_itbls	(fut_itbl_p FAR*, KpHandle_t FAR*);
static KpHandle_t	fut_unlock_chan		(fut_chan_p);

static KpGenericPtr_t
	fut_malloc (	KpInt32_t	size)
{
KpGenericPtr_t	ptr;

	ptr = allocBufferPtr (size);
	if (ptr != NULL)
		KpMemSet (ptr, 0, size);

	return (ptr);
}


/* convenient allocators of fut, and table structures:
 */
fut_p
	fut_alloc_fut (void)
{
fut_p	fut;

			/* allocate a zeroed block of memory */
	fut = (fut_p) fut_malloc((KpInt32_t) sizeof(fut_t));
 	if (fut == NULL) {
		return (NULL);
	}

	fut->magic = FUT_MAGIC;			/* set magic number */
	fut->refNum = fut_unique_id ();	/* and unique reference number */

			/* get handle and store */
	fut->handle = getHandleFromPtr ((KpGenericPtr_t)fut);
	return(fut);
}

fut_chan_p
	fut_alloc_chan (void)
{
fut_chan_p chan;

			/* allocate a zeroed block of memory */
	chan = (fut_chan_p)fut_malloc((KpInt32_t)sizeof(fut_chan_t));
 	if (chan == NULL) {
		return(NULL);
	}

			/* set magic number */
	chan->magic = FUT_CMAGIC;

			/* get handle and store */
	chan->handle = getHandleFromPtr ((KpGenericPtr_t)chan);
	return(chan);
}


fut_itbl_p
	fut_alloc_itbl (void)
{
fut_itbl_p itbl;

			/* allocate a zeroed block of memory */
	itbl = (fut_itbl_p )fut_malloc((KpInt32_t)sizeof(fut_itbl_t));
 	if ( itbl == NULL ) {
		return (NULL);
	}

	itbl->magic = FUT_IMAGIC;	/* set magic number */
	itbl->ref = 0;				/* and reference count */

			/* get handle and store */
	itbl->handle = getHandleFromPtr ((KpGenericPtr_t)itbl);
	
	return (itbl);
}


fut_gtbl_p
	fut_alloc_gtbl (void)
{
fut_gtbl_p  gtbl;

			/* allocate a zeroed block of memory */
	gtbl = (fut_gtbl_p )fut_malloc((KpInt32_t)sizeof(fut_gtbl_t));
 	if ( gtbl == NULL ) {
		return (NULL);
	}

	gtbl->magic = FUT_GMAGIC;	/* set magic number */
	gtbl->ref = 0;				/* and reference count */

			/* get handle and store */
	gtbl->handle = getHandleFromPtr ((KpGenericPtr_t)gtbl);
	
	return (gtbl);
}


fut_otbl_p
	fut_alloc_otbl (void)
{
fut_otbl_p  otbl;

			/* allocate a zeroed block of memory */
	otbl = (fut_otbl_p )fut_malloc((KpInt32_t)sizeof(fut_otbl_t));
 	if ( otbl == NULL ) {
		return (NULL);
	}

	otbl->magic = FUT_OMAGIC;	/* set magic number */
	otbl->ref = 0;				/* and reference count */

			/* get handle and store */
	otbl->handle = getHandleFromPtr ((KpGenericPtr_t)otbl);
	
	return (otbl);
}


fut_itbldat_p
	fut_alloc_itbldat (	fut_itbl_p	itbl)
{
KpInt32_t	size;

	if (! IS_ITBL(itbl)) {
		return NULL;
	}

	size = (FUT_INPTBL_ENT+1) * sizeof (fut_itbldat_t);

	itbl->tbl = (fut_itbldat_p) allocBufferPtr (size);
	if (itbl->tbl != NULL) {
		itbl->tblHandle = getHandleFromPtr ((KpGenericPtr_t)itbl->tbl);
	}
	else {
		itbl->tblHandle = NULL;
	}
	
	return itbl->tbl;
}


fut_gtbldat_p
	fut_alloc_gtbldat (	fut_gtbl_p	gtbl)
{
	if (! IS_GTBL(gtbl)) {
		return NULL;
	}

	gtbl->tbl = (fut_gtbldat_p) allocBufferPtr (gtbl->tbl_size);
	if (gtbl->tbl != NULL) {
		gtbl->tblHandle = getHandleFromPtr ((KpGenericPtr_t)gtbl->tbl);
	}
	else {
		gtbl->tblHandle = NULL;
	}
	
	return gtbl->tbl;
}


fut_otbldat_p
	fut_alloc_otbldat (	fut_otbl_p	otbl)
{
KpInt32_t	size;

	if (! IS_OTBL(otbl)) {
		return NULL;
	}

	size = (FUT_OUTTBL_ENT) * sizeof (fut_otbldat_t);

	otbl->tbl = (fut_otbldat_p) allocBufferPtr (size);
	if (otbl->tbl != NULL) {
		otbl->tblHandle = getHandleFromPtr ((KpGenericPtr_t)otbl->tbl);
	}
	else {
		otbl->tblHandle = NULL;
	}
	
	return otbl->tbl;
}


mf2_tbldat_p
	fut_alloc_imftdat (	fut_itbl_p	itbl,
						KpInt32_t	nEntries)
{
	if (! IS_ITBL(itbl)) {
		return NULL;
	}

	itbl->refTbl = (mf2_tbldat_p) allocBufferPtr (nEntries * sizeof (mf2_tbldat_t));

	if (itbl->refTbl != NULL) {
		itbl->refTblEntries = nEntries;
		itbl->refTblHandle = getHandleFromPtr ((KpGenericPtr_t)itbl->refTbl);
	}
	else {
		itbl->refTblEntries = 0;
		itbl->refTblHandle = NULL;
	}
	
	return itbl->refTbl;
}


mf2_tbldat_p
	fut_alloc_gmftdat (	fut_gtbl_p	gtbl)
{
KpInt32_t	theSize;

	if (! IS_GTBL(gtbl)) {
		return NULL;
	}

	theSize = (gtbl->tbl_size * sizeof (mf2_tbldat_t)) / sizeof (fut_gtbldat_t);
	
	gtbl->refTbl = (mf2_tbldat_p) allocBufferPtr (theSize);
	if (gtbl->refTbl != NULL) {
		gtbl->refTblHandle = getHandleFromPtr ((KpGenericPtr_t)gtbl->refTbl);
	}
	else {
		gtbl->refTblHandle = NULL;
	}
	
	return gtbl->refTbl;
}


mf2_tbldat_p
	fut_alloc_omftdat (	fut_otbl_p	otbl,
						KpInt32_t	nEntries)
{
	if (! IS_OTBL(otbl)) {
		return NULL;
	}

	otbl->refTbl = (mf2_tbldat_p) allocBufferPtr (nEntries * sizeof (mf2_tbldat_t));
	if (otbl->refTbl != NULL) {
		otbl->refTblEntries = nEntries;
		otbl->refTblHandle = getHandleFromPtr ((KpGenericPtr_t)otbl->refTbl);
	}
	else {
		otbl->refTblEntries = 0;
		otbl->refTblHandle = NULL;
	}
	
	return otbl->refTbl;
}


/* The fut lock and unlock functions are performed by these basic routines */

KpHandle_t
	fut_unlock_fut(fut_p fut)
{
KpInt32_t	i;
KpHandle_t	handle;

	if ( ! IS_FUT(fut)) {
		return NULL;
	}

	fut_unlock_itbls (fut->itbl, fut->itblHandle);	/* unlock itbls */

	for (i=0; i<FUT_NOCHAN; i++) {
		fut->chanHandle[i] = fut_unlock_chan (fut->chan[i]);
		#if defined(KPMAC)
		fut->chan[i] = FUT_NULL_CHAN;		/* make sure unlocked memory is not used */
		#endif
	}
	handle = getHandleFromPtr((KpGenericPtr_t)fut);

	(void)unlockBuffer (handle);

	return (handle);
}


static KpHandle_t
	fut_unlock_chan(fut_chan_p chan)
{
KpHandle_t	handle;
fut_gtbl_p	gtbl;
fut_otbl_p	otbl;

	if ( ! IS_CHAN(chan)) {
		return NULL;
	}

	fut_unlock_itbls (chan->itbl, chan->itblHandle);	/* unlock itbls */

	/* unlock gtbl */
	gtbl = chan->gtbl;

	if (IS_GTBL(gtbl)) {
		chan->gtblHandle = getHandleFromPtr((KpGenericPtr_t)gtbl);

		if (gtbl->tbl != NULL) {
			gtbl->tblHandle = getHandleFromPtr((KpGenericPtr_t)gtbl->tbl);
		}
		(void)unlockBuffer (gtbl->tblHandle);
		#if defined(KPMAC)
		gtbl->tbl = FUT_NULL_GTBLDAT;
		#endif

		if (gtbl->refTbl != NULL) {
			gtbl->refTblHandle = getHandleFromPtr((KpGenericPtr_t)gtbl->refTbl);
		}
		(void)unlockBuffer (gtbl->refTblHandle);
		#if defined(KPMAC)
		gtbl->refTbl = NULL;
		#endif

		(void)unlockBuffer (chan->gtblHandle);
	}

	#if defined(KPMAC)
	chan->gtbl = FUT_NULL_GTBL;
	#endif

	/* unlock otbl */
	otbl = chan->otbl;

	if (IS_OTBL(otbl)) {
		chan->otblHandle = getHandleFromPtr((KpGenericPtr_t)otbl);

		if (otbl->tbl != NULL) {
			otbl->tblHandle = getHandleFromPtr((KpGenericPtr_t)otbl->tbl);
		}
		(void)unlockBuffer (otbl->tblHandle);
		#if defined(KPMAC)
		otbl->tbl = FUT_NULL_OTBLDAT;
		#endif

		if (otbl->refTbl != NULL) {
			otbl->refTblHandle = getHandleFromPtr((KpGenericPtr_t)otbl->refTbl);
		}
		(void)unlockBuffer (otbl->refTblHandle);
		#if defined(KPMAC)
		otbl->refTbl = NULL;
		#endif

		(void)unlockBuffer (chan->otblHandle);
	}

	#if defined(KPMAC)
	chan->otbl = FUT_NULL_OTBL;
	#endif

	/* unlock the chan structure */
	handle = getHandleFromPtr((KpGenericPtr_t)chan);

	(void)unlockBuffer (handle);

	return handle;
}


static void
	fut_unlock_itbls (fut_itbl_p FAR* itbls, KpHandle_t FAR* itblsH)
{
fut_itbl_p	itbl;
KpInt32_t	i;

	for (i = 0; i < FUT_NICHAN; i++) {
		itbl = itbls[i];
		
		if (IS_ITBL(itbl)) {
			itblsH[i] = getHandleFromPtr((KpGenericPtr_t)itbl);

			if (itbl->tbl != NULL) {
				itbl->tblHandle = getHandleFromPtr((KpGenericPtr_t)itbl->tbl);
			}
			(void)unlockBuffer (itbl->tblHandle);
			#if defined(KPMAC)
			itbl->tbl = FUT_NULL_ITBLDAT;
			#endif

			if (itbl->refTbl != NULL) {
				itbl->refTblHandle = getHandleFromPtr((KpGenericPtr_t)itbl->refTbl);
			}
			(void)unlockBuffer (itbl->refTblHandle);
			#if defined(KPMAC)
			itbl->refTbl = NULL;
			#endif

			(void)unlockBuffer (itblsH[i]);
		}

		#if defined(KPMAC)
		itbls[i] = FUT_NULL_ITBL;
		#endif
	}
}


fut_p
	fut_lock_fut (KpHandle_t handle)
{
KpInt32_t	i;
fut_p		fut;

	if (handle == NULL) {
		return (FUT_NULL);
	}

	fut = (fut_p) lockBuffer (handle);

	fut_lock_itbls (fut->itbl, fut->itblHandle);	/* lock itbls */

	for (i = 0; i < FUT_NOCHAN; i++) {
		fut->chan[i] = fut_lock_chan(fut->chanHandle[i]);
	}
	return fut;
}


static fut_chan_p
	fut_lock_chan (KpHandle_t handle)
{
fut_chan_p	chan;
fut_gtbl_p	gtbl;
fut_otbl_p	otbl;

	if (handle == NULL) {
		return (FUT_NULL_CHAN);
	}

	chan = (fut_chan_p) lockBuffer (handle);

	fut_lock_itbls (chan->itbl, chan->itblHandle);	/* lock itbls */

	/* lock gtbl */
	if (chan->gtblHandle == NULL) {
		gtbl = FUT_NULL_GTBL;
	}
	else {
		gtbl = (fut_gtbl_p )lockBuffer (chan->gtblHandle);
		gtbl->tbl = (fut_gtbldat_p)lockBuffer (gtbl->tblHandle);
		gtbl->refTbl = (mf2_tbldat_p )lockBuffer (gtbl->refTblHandle);
	}

	chan->gtbl = gtbl;

	/* lock otbl */
	if (chan->otblHandle == NULL) {
		otbl = FUT_NULL_OTBL;
	}
	else {
		otbl = (fut_otbl_p )lockBuffer (chan->otblHandle);
		otbl->tbl = (fut_otbldat_p)lockBuffer (otbl->tblHandle);
		otbl->refTbl = (mf2_tbldat_p )lockBuffer (otbl->refTblHandle);
	}

	chan->otbl = otbl;

	return chan;
}


static void
	fut_lock_itbls (fut_itbl_p FAR* itbls, KpHandle_t FAR* itblsH)
{
KpInt32_t	i;
KpHandle_t	handle;
fut_itbl_p	itbl;

	for (i = 0; i < FUT_NICHAN; i++) {
		handle = itblsH[i];
		if (handle == NULL) {
			itbl = FUT_NULL_ITBL;
		}
		else {
			itbl = (fut_itbl_p )lockBuffer (handle);
			itbl->tbl = (fut_itbldat_p) lockBuffer (itbl->tblHandle);
			itbl->refTbl = (mf2_tbldat_p) lockBuffer (itbl->refTblHandle);
		}
		
		itbls[i] = itbl;
	}
}
