/*
 * @(#)resize.c	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)resize.c	1.5 99/02/12

	Contains:	function to change grid table sizes without changing
			the function.

	Author:		George Pawle

	COPYRIGHT (c) 1991-1999 Eastman Kodak Company.
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */

#include "kcptmgr.h"
#include "fut_util.h"		/* internal interface file */


fut_p
	fut_resize (	fut_p		fut,
					KpInt32_p	sizeArray)
{
fut_p		reSizedGtblFut = NULL, gtblFut = NULL, reSizedFut = NULL, identityFut = NULL;
KpInt32_t	i1, i2, iomask, iiomask, imask, omask, sameDims;
fut_chan_p	chan;
fut_itbl_p	itbl, itbls[FUT_NICHAN];
fut_gtbl_p	gtbls[FUT_NOCHAN];
fut_otbl_p	otbls[FUT_NOCHAN];

	#if defined KCP_DIAG_LOG
	kcpDiagLog ("fut_resize\n");
	#endif
	
	if ( ! IS_FUT(fut)) {
		return NULL;
	}

	for (i1 = 0; i1 < FUT_NICHAN; i1++) {
		itbls[i1] = FUT_NULL_ITBL;			/* init to null for freeing on error */
	}

	/* collect the gtbls from the source fut */
	/* make sure that all the gtbls use the same itbls */
	omask = 0;
	
	for (i1 = 0, sameDims = 1; i1 < FUT_NOCHAN; i1++) {
		chan = fut->chan[i1];
		if (IS_CHAN(chan)) {
			for (i2 = 0; i2 < FUT_NICHAN; i2++) {
				itbl = fut->itbl[i2];
				if (chan->itbl[i2] != itbl) {	/* must be shared */
					goto GetOut;
				}
				
				if (IS_ITBL(itbl)) {
					if (itbl->size != sizeArray [i2]) {
						sameDims = 0;			/* not the same */
					}
				}
			}

			omask |= FUT_BIT(i1);			/* resize this chan */
			gtbls[i1] = chan->gtbl;			/* collect gtbls */
		}
		else {
			gtbls[i1] = NULL;
		}
	}

	if (sameDims == 1) {
		return fut;		/* already the right size! */
	}

	imask = fut->iomask.in;
	iomask = FUT_OUT(omask) | FUT_IN(imask);
	
	/* make a new fut with these gtbls and identity itbls and otbls */
	gtblFut = fut_new (iomask, NULL, gtbls, NULL);
	if (gtblFut != NULL) {

		/* make an identity fut with itbls that have the specified sizes */
		iiomask = FUT_OUT(imask) | FUT_IN(imask);
		identityFut = constructfut (iiomask, sizeArray, NULL, NULL, NULL, NULL, KCP_FIXED_RANGE, KCP_FIXED_RANGE);
		if (identityFut != NULL) {
		
			/* compose the new size fut with the gtbl fut */
			reSizedGtblFut = fut_comp (gtblFut, identityFut, 0);
			if (reSizedGtblFut != NULL) {

				/* make a new fut with original itbls, ... */
				for (i1 = 0; i1 < FUT_NICHAN; i1++) {
					if ((imask & FUT_BIT(i1)) != 0) {
						itbls[i1] = fut_copy_itbl (fut->itbl[i1]);			/* copy (do not share!) original itbls */
						if (itbls[i1] == NULL) {
							goto GetOut;
						}
						makeMftiTblDat (itbls[i1]);							/* convert to mft to remove grid size dependancy */
						itbls[i1]->size = reSizedGtblFut->itbl[i1]->size;	/* set new grid size */
						fut_free_itbldat (itbls[i1], freeData);				/* free fixed table, it has incorrect grid indices */
					}
				}

				/* ... resized gtbls, and original otbls */
				for (i1 = 0; i1 < FUT_NOCHAN; i1++) {
					if ((omask & FUT_BIT(i1)) != 0) {
						gtbls[i1] = reSizedGtblFut->chan[i1]->gtbl;	/* collect resized gtbls */
						otbls[i1] = fut->chan[i1]->otbl;			/* and original otbls */
					}
					else {
						gtbls[i1] = NULL;
						otbls[i1] = NULL;
					}
				}

				reSizedFut = fut_new (iomask, itbls, gtbls, otbls);
			}
		}
	}

GetOut:
	fut_free (reSizedGtblFut);	/* free the intermediate futs */
	fut_free (gtblFut);
	fut_free (identityFut);
	fut_free_tbls (FUT_NICHAN, (void **)itbls);

	return (reSizedFut);
}
