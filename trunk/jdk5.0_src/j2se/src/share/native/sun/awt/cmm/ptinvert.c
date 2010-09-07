/*
 * @(#)ptinvert.c	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)ptinvert.c	1.8 98/09/09

	Contains:	This module contains an invert function for
				Color Processor Transforms (RefNums)

				Created by lsh, Feb. 5, 1994
				Moved to the Color Processor by
				doro, May 5, 1998

	Written by:	The Kodak CMS Team

	COPYRIGHT (c) 1993-1998 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include "kcmptlib.h"
#include "kcptmgr.h"
#include "fut.h"
#include "fut_util.h"
#include "attrib.h"
#include <stdio.h>


/*--------------------------------------------------------------------
 * DESCRIPTION
 * This function determines if a table handle has been
 * processed before.
 *
 * AUTHOR
 * PGT
 *
 * DATE CREATED
 * May 12, 1994
 *--------------------------------------------------------------------*/
static KpBool_t
	UniqueTable (	KcmHandle	tbl,
					KcmHandle	FAR *tblList,
					KpInt32_t	numTbls)
{
KpInt32_t	i;

	for (i = 0; i < numTbls; ++i) {
		if (tbl == tblList [i]) {
			return   KPFALSE;
		}
	}

	return KPTRUE;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 * This function checks to determine if the PT can be inverted
 *
 * AUTHOR
 * PGT
 *
 * DATE CREATED
 * May 11, 1994
 *--------------------------------------------------------------------*/
static KpBool_t
	InvertOk (	PTRefNum_t	RefNum,
				KpInt32_t	senseAttrib)
{
KpInt32_t	sense;
KpInt32_t	invertAttrib;
PTErr_t		PTErr;
KpChar_t	AttrValue [10];
KpInt32_t	AttrSize;

/* See if the attribute is the right type */
	switch (senseAttrib) {
	case KCM_MEDIUM_SENSE_IN:
		invertAttrib = KCM_SENSE_INVERTIBLE_IN;
		break;

	case KCM_MEDIUM_SENSE_OUT:
		invertAttrib = KCM_SENSE_INVERTIBLE_OUT;
		break;

	default:
		return KPFALSE;
	}

/* If the sense attribute is not in this PT, then this PT cannot be
 * inverted for that sense because it is a color space for which
 * inverting is not appropriate (e.g., RCS) */
	AttrSize = sizeof(AttrValue);
	PTErr = PTGetAttribute (RefNum, senseAttrib, &AttrSize, AttrValue);
	if (PTErr != KCP_SUCCESS) {
		return KPFALSE;
	}

	sense = KpAtoi (AttrValue);
	if (KCM_UNKNOWN == sense) {
		return KPFALSE;
	}

/* Make sure that the PT is allowed to be inverted:
 *  if the attribute KCM_SENSE_INVERTIBLE==KCM_IS_INVERTIBLE
 *  if KCM_SENSE_INVERTIBLE is missing and KCM_CLASS==KCM_OUTPUT_CLASS */
	AttrSize = sizeof(AttrValue);
	PTErr = PTGetAttribute (RefNum, invertAttrib, &AttrSize, AttrValue);
	/* If attribute not set, OK to invert */
	if (PTErr == KCP_SUCCESS) {
		switch ( KpAtoi (AttrValue)) {
		case KCM_IS_NOT_INVERTIBLE:
			return KPFALSE;

		case KCM_IS_INVERTIBLE:
			return KPTRUE;
		}
	}

	AttrSize = sizeof(AttrValue);
	PTErr = PTGetAttribute (RefNum, KCM_CLASS, &AttrSize, AttrValue);
	if (PTErr != KCP_SUCCESS) {
		return KPFALSE;
	}
	
	if (KCM_OUTPUT_CLASS == KpAtoi(AttrValue)) {
		return KPTRUE;
	}

	return KPFALSE;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 * This function inverts the output tables of a PT
 *
 * AUTHOR
 * PGT
 *
 * DATE CREATED
 * May 11, 1994
 *--------------------------------------------------------------------*/
static PTErr_t
	InvertOutputTables (	PTRefNum_t	refNum,
							KpInt32_t	numOutChan)
{
KpInt32_t	i, k;
PTErr_t		cpStatus;
KcmHandle	refTblHandle;
KcmHandle	tblList[FUT_NOCHAN];
mf2_tbldat_p	refTbl;
KpInt32_t	numTbls, refTblEntries, dummy = 0;

	/* Get each table, invert if not already inverted */
	cpStatus = KCP_SUCCESS;
	numTbls = 0;
	for (i = 0; (i < numOutChan) && (cpStatus == KCP_SUCCESS); ++i) {
		cpStatus = getRefTbl (FUT_OMAGIC, refNum, dummy, i, &refTblHandle, &refTblEntries);
		if (cpStatus == KCP_NO_OUTTABLE) {
			cpStatus = KCP_SUCCESS;
		}
		else {
			if ((cpStatus == KCP_SUCCESS) && (UniqueTable (refTblHandle, tblList, numTbls))) {
				refTbl = lockBuffer (refTblHandle);
				if (refTbl != NULL) {
					tblList [numTbls++] = refTblHandle;	/* save address to avoid multiple inversion */

					for (k = 0; k < refTblEntries; ++k ) {		/* invert the table */
						refTbl[k] = (mf2_tbldat_t)(MF2_TBL_MAXVAL - refTbl[k]);	/* by negating the contents of each entry */
					}

					unlockBuffer (refTblHandle);
				}
				else {
					cpStatus = KCP_MEM_LOCK_ERR;
				}
			}
		}
	}

	return cpStatus;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 * This function inverts the input tables of a PT
 *
 * AUTHOR
 * PGT
 *
 * DATE CREATED
 * May 11, 1994
 *--------------------------------------------------------------------*/
static PTErr_t
	InvertInputTables (	PTRefNum_t	refNum,
						KpInt32_t	numOutChan,
						KpInt32_t	FAR *numInVar)
{
KpInt32_t	in, out, k;
PTErr_t		cpStatus;
KcmHandle	refTblHandle;
mf2_tbldat_p	refTbl;
mf2_tbldat_t	refTblTemp;
KcmHandle	tblList[FUT_NOCHAN * FUT_NICHAN];
KpInt32_t	numTbls, refTblEntries;

	/* Get each table, invert if not already inverted */
	cpStatus = KCP_SUCCESS;
	for (out = 0, numTbls = 0; (out < numOutChan) && (cpStatus == KCP_SUCCESS); ++out) {
		for (in = 0; (in < numInVar [out]) && (cpStatus == KCP_SUCCESS); ++in) {
			cpStatus = getRefTbl (FUT_IMAGIC, refNum, in, out, &refTblHandle, &refTblEntries);
			if (cpStatus == KCP_NO_INTABLE)	{	/* no input tbl, ignore */
				cpStatus = KCP_SUCCESS;
			}
			else {
				if ((cpStatus == KCP_SUCCESS) && (UniqueTable (refTblHandle, tblList, numTbls))) {
					refTbl = lockBuffer (refTblHandle);
					if (refTbl != NULL) {
						tblList [numTbls++] = refTblHandle;	/* save address to avoid multiple inversion */

						for (k = 0; k < refTblEntries/2; ++k) {		/* invert the table */
							refTblTemp = refTbl [k];				/* by reversing the order of the entries */
							refTbl [k] = refTbl [refTblEntries-1-k];
							refTbl [refTblEntries-1-k] = refTblTemp;
						}

						unlockBuffer (refTblHandle);
					}
					else {
						cpStatus = KCP_MEM_LOCK_ERR;
					}
				}
			}
		}
	}
	
	return cpStatus;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 * This function obtains the number of input and output
 * channels of a PT
 *
 * AUTHOR
 * PGT
 *
 * DATE CREATED
 * May 11, 1994
 *--------------------------------------------------------------------*/
static PTErr_t
	getNumChans (	PTRefNum_t	RefNum,
					KpInt32_p	numOutChan,
					KpInt32_p	numInVar)
{
KpInt32_t	attrib[FUT_NOCHAN] = {KCM_NUM_VAR_1_IN, KCM_NUM_VAR_2_IN, KCM_NUM_VAR_3_IN,
				KCM_NUM_VAR_4_IN, KCM_NUM_VAR_5_IN, KCM_NUM_VAR_6_IN, KCM_NUM_VAR_7_IN, KCM_NUM_VAR_8_IN};
KpInt32_t	i, nOutChan;
PTErr_t		PTErr;
KpChar_t	AttrValue [10];
KpInt32_t	AttrSize;

/* get number of output variables and the number of input variables for each output variable */
	AttrSize = sizeof (AttrValue);
	PTErr = PTGetAttribute (RefNum, KCM_NUM_VAR_OUT, &AttrSize, AttrValue);
	if (PTErr != KCP_SUCCESS) {
		return PTErr;
	}

	nOutChan = KpAtoi (AttrValue);
	*numOutChan = nOutChan;
	if (nOutChan > FUT_NOCHAN) {
		nOutChan = FUT_NOCHAN;
	}

	for (i = 0; i < nOutChan; i++) {
		AttrSize = sizeof (AttrValue);
		PTErr = PTGetAttribute(RefNum, attrib[i], &AttrSize, AttrValue);
		if (KCP_SUCCESS != PTErr) {
			numInVar [i] = 0;
		}
		else {
			numInVar [i] = KpAtoi (AttrValue);
		}
	}

	return KCP_SUCCESS;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 * This function inverts the sense of either the input or
 * output table.  The attribute senseAttrib can be either 
 * KCM_MEDIUM_SENSE_IN or KCM_MEDIUM_SENSE_OUT
 *
 * AUTHOR
 * PGT    Originally named InvertXformSense from spxfromr.c
 *
 * DATE CREATED
 * May 11, 1994
 *
 * Date Moved
 * May 5, 1998
 *--------------------------------------------------------------------*/
PTErr_t
	PTInvert (	PTRefNum_t	RefNum,
				KpInt32_t	senseAttrib)
{
KpInt32_t	numInVar [FUT_NICHAN];
KpInt32_t	numOutChan;
PTErr_t		PTStatus;

/* Check for proper parameters */
	if (!InvertOk (RefNum, senseAttrib)) {
		return KCP_INVAL_OPREFNUM;
	}

/* Find out the number of channels in the PT */
	PTStatus = getNumChans (RefNum, &numOutChan, numInVar);
	if (PTStatus == KCP_SUCCESS) {
		switch (senseAttrib) {
		case KCM_MEDIUM_SENSE_IN:
			return InvertInputTables (RefNum, numOutChan, numInVar);

		case KCM_MEDIUM_SENSE_OUT:
			return InvertOutputTables (RefNum, numOutChan);

		default:
			return KCP_BAD_ARG;
		}
	}

	return PTStatus;
}

