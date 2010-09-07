/*
 * @(#)futiotbl.c	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)futiotbl.c	2.35 99/03/04

	Contains:	Table access functions for futs.

	These routines provide access to the individual tables of a fut.
	Since there is little protection against abuse, great care should
	be exercised when using them.

	Windows Revision Level:
		$Workfile:  $
		$Logfile:  $
		$Revision:  $
		$Date:  $
		$Author:  $
		
	COPYRIGHT (c) 1992-2000 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */

#include "fut.h"
#include "fut_util.h"
#include "kcmptlib.h"
#include "kcms_sys.h"
#include "kcptmgrd.h"
#include "kcptmgr.h"

/* prototypes */
static PTErr_t getTbl(KpInt32_t, PTRefNum_t,
				KpInt32_t, KpInt32_t, KpInt32_p, KpInt32_p, KpHandle_t FAR*);


PTErr_t
	PTNewEmpty (KpInt32_t	ndim,
				KpInt32_p	dim,
				KpInt32_t	nchan,
				PTRefNum_p	PTRefNum)
{
PTErr_t	errnum = KCP_BAD_ARG;
fut_p fut;

	if (PTRefNum == NULL) return (KCP_BAD_PTR);
	if (dim == NULL) return (KCP_BAD_PTR);

	fut = fut_new_empty (ndim, dim, nchan, KCP_UNKNOWN, KCP_UNKNOWN);
	if (fut != NULL) {
		if (mft_to_fut (fut) != 1) {		/* convert to fut tables */
			fut_free (fut);
			fut = NULL;
		} else {
			errnum = fut2PT (&fut, -1, -1, PTTYPE_CALCULATED, PTRefNum);	/* make into PT */
		}
	}
	return (errnum);
}


PTErr_t
	PTNewEmptySep (	KpInt32_t	nchan,
					KpInt32_p	dim,
					PTRefNum_p	PTRefNum)
{
PTErr_t		errnum;
fut_p		fut;
KpInt32_t	iomask, i1;
fut_itbl_p	itbl;
fut_gtbl_p	gtbl;
fut_otbl_p	otbl;
fut_calcData_t	data;

	if ((nchan > FUT_NICHAN) || (nchan > FUT_NOCHAN)) return KCP_BAD_ARG;
	if (PTRefNum == NULL) return (KCP_BAD_PTR);
	if (dim == NULL) return (KCP_BAD_PTR);

	fut = fut_new (0, NULL, NULL, NULL);	/* make a fut with nothing in it */

	for (i1 = 0; i1 < nchan; i1++) {
		iomask = FUT_IN(FUT_BIT(i1)) | FUT_OUT(FUT_BIT(i1));
		data.chan = i1;						/* define channel */

		itbl = fut_new_itblEx (KCP_REF_TABLES, KCP_FIXED_RANGE, dim[i1], fut_irampEx, NULL);			/* make input table */
		gtbl = fut_new_gtblEx (KCP_REF_TABLES, iomask, fut_grampEx, &data, dim);	/* make grid table */
		otbl = fut_new_otblEx (KCP_REF_TABLES, KCP_FIXED_RANGE, fut_orampEx, NULL);					/* make output tables */

		itbl->dataClass = KCP_FIXED_RANGE;	/* separable can not be PCS */
		otbl->dataClass = KCP_FIXED_RANGE;

		if ( ! fut_defchan(fut, iomask, &itbl, gtbl, otbl) ) {
			fut_free (fut);
			return (KCP_BAD_ARG);
		}

/* since tables are shared in the fut library,
 * free the original tables since they are still 'ours' */
		fut_free_itbl (itbl);
		fut_free_gtbl (gtbl);
		fut_free_otbl (otbl);
	}

	errnum = fut2PT (&fut, -1, -1, PTTYPE_CALCULATED, PTRefNum);	/* make into PT */
	
	return (errnum);
}


PTErr_t
	PTGetItbl (	PTRefNum_t		PTRefNum,
				KpInt32_t		ochan,
				KpInt32_t		ichan,
				KpHandle_t FAR*	itblDat)
{
PTErr_t		errnum;
KpInt32_p	nDim = NULL;
KpInt32_p	dimList = NULL;

	if (itblDat == NULL) return (KCP_BAD_PTR);

	errnum = getTbl (FUT_IMAGIC, PTRefNum, ochan, ichan, nDim, dimList, itblDat);

	return (errnum);
}


PTErr_t 
	PTGetGtbl (	PTRefNum_t		PTRefNum,
				KpInt32_t		ochan,
				KpInt32_p		nDim,
				KpInt32_p		dimList,
				KpHandle_t FAR*	gtblDat)
{
PTErr_t	errnum;
KpInt32_t ichan = -1;

	if (gtblDat == NULL) return (KCP_BAD_PTR);
	if (nDim == NULL) return (KCP_BAD_PTR);
	if (dimList == NULL) return (KCP_BAD_PTR);	/* dimList must have at least 4 entries */

	errnum = getTbl (FUT_GMAGIC, PTRefNum, ochan, ichan, nDim, dimList, gtblDat);

	return (errnum);
}


PTErr_t
	PTGetOtbl (	PTRefNum_t		PTRefNum,
				KpInt32_t		ochan,
				KpHandle_t FAR*	otblDat)
{
PTErr_t		errnum;
KpInt32_t	ichan = -1;
KpInt32_p	nDim = NULL, dimList = NULL;

	if (otblDat == NULL) return (KCP_BAD_PTR);

	errnum = getTbl (FUT_OMAGIC, PTRefNum, ochan, ichan, nDim, dimList, otblDat);

	return (errnum);
}


/* Get a fut table
 * if the PT is active:
 *		get the fut address, lock it, get the requested table,
 *		get the handle to the table, unlock the fut
 * Returns:
 *   KCP_SUCCESS or table failure state 
 */
static PTErr_t
	getTbl(	KpInt32_t	tblSel,
			PTRefNum_t	PTRefNum,
			KpInt32_t	ochan,
			KpInt32_t	ichan,
			KpInt32_p	nDim,
			KpInt32_p	dimList,
			KpHandle_t FAR*	tblH)
{
PTErr_t			errnum, errnum1;
KpGenericPtr_t	PTHdr;
PTAddr_t		FAR* PTHdrH, FAR* PTDataH;
KpGenericPtr_t	tblP;
fut_p			fut;
fut_gtbl_p		gtbl;
KpInt32_t		i1, fut_err = 0;

	errnum = PTGetPTInfo (PTRefNum, &PTHdrH, NULL, &PTDataH);	/* get PT info */

	if ((errnum == KCP_PT_ACTIVE) || (errnum == KCP_SERIAL_PT)) {
		freeEvalTables (PTRefNum);	/* clear optimized tables to force rebuild */

		errnum = initExport ((KpHandle_t)PTHdrH, (KpHandle_t)PTDataH, PTTYPE_FUTF, (fut_hdr_p FAR *)&PTHdr, &fut);	/* set up to export the data */
		if (errnum != KCP_SUCCESS) {
			return errnum;
		}

		fut_free_mftdat (fut);	/* free the matrix-fut data tables to force rebuild */

		switch (tblSel) {
			case FUT_IMAGIC:
				fut_err = fut_get_itbl (fut, ochan, ichan, (fut_itbldat_h)&tblP);
				if (fut_err != 1) {
					errnum = KCP_NO_INTABLE;
				}
				break;
				
			case FUT_GMAGIC:
				fut_err = fut_get_gtbl (fut, ochan, (fut_gtbldat_h)&tblP);
				if (fut_err == 1) {
					gtbl = fut->chan[ochan]->gtbl;	/* get grid table structure */

					for (i1 = 0, *nDim = 0; i1 < FUT_NICHAN; i1++) {	/* find the active dimensions of the grid */
						if (gtbl->size[i1] > 1) {
							dimList[*nDim] = gtbl->size[i1];			/* return dimension sizes */
							(*nDim)++;									/* return # of input variables */
						}
					}
				}
				else {				
					errnum = KCP_INVAL_GRID_DIM;
				}
				break;
				
			case FUT_OMAGIC:
				fut_err = fut_get_otbl (fut, ochan, (fut_otbldat_h)&tblP);
				if (fut_err != 1) {
					errnum = KCP_NO_OUTTABLE;
				}
				break;
		}
		
		if (errnum == KCP_SUCCESS) {
		
		
			*tblH = getHandleFromPtr(tblP);	/* return handle to data */
		
			if (!fut_io_encode (fut, PTHdr)) {	/* make the info header */
				errnum = KCP_ENCODE_PTHDR_ERR;
			}
			else {				
				errnum = KCP_SUCCESS;
			}
		}

		errnum1 = unlockPT ((KpHandle_t)PTHdrH, fut);
		if (errnum == KCP_SUCCESS) {
			errnum = errnum1;
		}
	}

	return (errnum);
}


/* Get a matrix fut table
 * if the PT is active:
 *		get the fut address, lock it, get the requested table,
 *		get the handle to the table, unlock the fut
 * Returns:
 *   KCP_SUCCESS or table failure state 
 */
PTErr_t
	getRefTbl(	KpInt32_t	tblSel,
				PTRefNum_t	PTRefNum,
				KpInt32_t	ichan,
				KpInt32_t	ochan,
				KpHandle_t *refTblHandle,
				KpInt32_p	refTblEntries)
{
PTErr_t			errnum;
PTAddr_t		FAR* PTHdrH, FAR* PTDataH;
fut_p			fut;
KpInt32_t		status = 1;

	errnum = PTGetPTInfo(PTRefNum, &PTHdrH, NULL, &PTDataH);	/* get PT info */

	if ((errnum == KCP_PT_ACTIVE) || (errnum == KCP_SERIAL_PT)) {
		errnum = KCP_SUCCESS;		/* let's assume everything is going to be OK */
		freeEvalTables (PTRefNum);	/* clear optimized tables to force rebuild */

	/* get fut pointer */
		fut = fut_lock_fut ((KpHandle_t)PTDataH);
		if ( ! IS_FUT(fut)) {
			return (KCP_PTERR_2);
		}

		switch (tblSel) {
			case FUT_IMAGIC:
				*refTblHandle = fut->chan[ochan]->itbl[ichan]->refTblHandle;
				*refTblEntries = fut->chan[ochan]->itbl[ichan]->refTblEntries;
				if (*refTblHandle == NULL) {
					/* Convert fut tables to Reference tables and then get info */
					status = fut_to_mft (fut);
					*refTblHandle = fut->chan[ochan]->itbl[ichan]->refTblHandle;
					*refTblEntries = fut->chan[ochan]->itbl[ichan]->refTblEntries;
				}
				if (status ==1) {
					fut_free_itbldat (fut->chan[ochan]->itbl[ichan], freeData);
				}
				if (*refTblHandle == NULL) {
					errnum = KCP_NO_INTABLE;
				}
				break;
								
			case FUT_OMAGIC:
				*refTblHandle = fut->chan[ochan]->otbl->refTblHandle;
				*refTblEntries = fut->chan[ochan]->otbl->refTblEntries;
				if (*refTblHandle == NULL) {
					status = fut_to_mft (fut);
					*refTblHandle = fut->chan[ochan]->otbl->refTblHandle;
					*refTblEntries = fut->chan[ochan]->otbl->refTblEntries;
				}
				if (status ==1) {
					fut_free_otbldat (fut->chan[ochan]->otbl, freeData);
				}
				if (*refTblHandle == NULL) {
					errnum = KCP_NO_OUTTABLE;
				}
				break;


			default:
				errnum = KCP_BAD_ARG;
		}

		fut_unlock_fut (fut);
		errnum = KCP_SUCCESS;
	}

	return (errnum);
}
